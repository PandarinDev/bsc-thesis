#!/usr/bin/env python3

import sys
import struct
import base64
from typing import Dict, Optional
from pathlib import Path
from dataclasses import dataclass
from enum import Enum

@dataclass
class Color:
    r: float
    g: float
    b: float

def parse_material_lib(path: Path, target_dict: Dict[str, Color]):
    current_material_name = None
    with open(path, "r") as file_handle:
        lines = file_handle.readlines()
        for raw_line in lines:
            line = raw_line.strip()
            # Skip comment lines
            if line.startswith("#"):
                continue
            if line.startswith("newmtl "):
                current_material_name = line.split(" ")[1]
            if line.startswith("Kd "):
                data = line.split(" ")
                target_dict[current_material_name] = Color(float(data[1]), float(data[2]), float(data[3]))

class ConversionMode(str, Enum):
    DYNAMIC = "dynamic"
    STATIC = "static"

# Converts OBJ files to binary representation that is expected by the program.
def main():
    args = sys.argv[1:]
    num_args = len(args)
    if num_args < 2:
        print("Usage: convert_obj.py <dynamic/static> <obj1> [<obj2> <...>]")
        sys.exit(1)
    mode = args[0]
    if mode != ConversionMode.DYNAMIC and mode != ConversionMode.STATIC:
        print("Conversion mode must be 'dynamic' or 'static'.")
        sys.exit(1)
    for file_path in args[1:]:
        print(f"Trying to convert '{file_path}'.")
        binary_data = bytearray()
        # Read the data from the input OBJ file and store it in the binary buffer
        with open(file_path, "r") as file_handle:
            lines = file_handle.readlines()
            vertices = []
            normals = []
            materials = {}
            material_name = ""
            for raw_line in lines:
                line = raw_line.strip()
                # Skip comment lines
                if line.startswith("#"):
                    continue
                # Material library
                if line.startswith("mtllib"):
                    mtllib_filename = line.split(" ")[1]
                    mtllib_path = Path(file_path).parent / mtllib_filename
                    parse_material_lib(mtllib_path, materials)
                # Vertex
                if line.startswith("v "):
                    data = line.split(" ")
                    vertices.append(tuple(float(data[i]) for i in range(1, 4)))
                # Normal
                elif line.startswith("vn "):
                    data = line.split(" ")
                    normals.append(tuple(float(data[i]) for i in range(1, 4)))
                # Material change
                elif line.startswith("usemtl "):
                    material_name = line.split(" ")[1]
                    # Only check for the material's existence in static conversion mode
                    if mode == ConversionMode.STATIC and material_name not in materials:
                        raise f"Unknown material: {material_name}"
                # Face
                elif line.startswith("f "):
                    # Currently the script assumes that there are no texture coordinates
                    data = line.split(" ")
                    for i in range(1, 4):
                        if not material_name:
                            raise "Face definition before material"
                        parts = data[i].split("//")
                        vertex_index = int(parts[0]) - 1
                        normal_index = int(parts[1]) - 1
                        vertex = vertices[vertex_index]
                        normal = normals[normal_index]
                        material_name_length = len(material_name)
                        if material_name_length > 255:
                            raise "Material name length cannot exceed 255 bytes"
                        if mode == ConversionMode.DYNAMIC:
                            binary_data.extend(struct.pack(
                                f"ffffffB{len(material_name)}s",
                                vertex[0], vertex[1], vertex[2],
                                normal[0], normal[1], normal[2],
                                material_name_length, material_name.encode()))
                        else:
                            current_material = materials[material_name]
                            binary_data.extend(struct.pack(
                                "fffffffff",
                                vertex[0], vertex[1], vertex[2],
                                normal[0], normal[1], normal[2],
                                current_material.r, current_material.g, current_material.b))
        # Write the result as a base64 encoded string to stdout
        print(base64.b64encode(binary_data))


if __name__ == "__main__":
    main()