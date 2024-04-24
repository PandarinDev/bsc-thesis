#!/usr/bin/env python3

import sys
import struct
import base64
from dataclasses import dataclass
from typing import Tuple
from enum import Enum

@dataclass(frozen=True)
class Vertex:
    position: Tuple[float, float, float]
    normal: Tuple[float, float, float]
    color: Tuple[float, float, float]

    def position_line(self) -> str:
        return f"v {self.position[0]} {self.position[1]} {self.position[2]}"

    def normal_line(self) -> str:
        return f"vn {self.normal[0]} {self.normal[1]} {self.normal[2]}"

@dataclass(frozen=True)
class VertexWithMaterial:
    position: Tuple[float, float, float]
    normal: Tuple[float, float, float]
    material: str

    def position_line(self) -> str:
        return f"v {self.position[0]} {self.position[1]} {self.position[2]}"

    def normal_line(self) -> str:
        return f"vn {self.normal[0]} {self.normal[1]} {self.normal[2]}"

class ConversionMode(str, Enum):
    DYNAMIC = "dynamic"
    STATIC = "static"

def unpack_dynamic_vertex(data: bytes, offset: int) -> Tuple[VertexWithMaterial, int]:
    floats_per_vertex_before_material = 6
    vertex_data = struct.unpack_from("f" * floats_per_vertex_before_material, data, offset)
    material_length_offset = offset + floats_per_vertex_before_material * 4
    material_length = struct.unpack_from("B", data, material_length_offset)[0]
    material = struct.unpack_from(f"{material_length}s", data, material_length_offset + 1)[0]
    return (VertexWithMaterial(
        (vertex_data[0], vertex_data[1], vertex_data[2]),
        (vertex_data[3], vertex_data[4], vertex_data[5]),
        material), floats_per_vertex_before_material * 4 + 1 + material_length)

def unpack_static_vertex(data: bytes, offset: int) -> Tuple[Vertex, int]:
    floats_per_vertex = 9
    vertex_data = struct.unpack_from("f" * floats_per_vertex, data, offset)
    return (Vertex(
        (vertex_data[0], vertex_data[1], vertex_data[2]),
        (vertex_data[3], vertex_data[4], vertex_data[5]),
        (vertex_data[6], vertex_data[7], vertex_data[8])), floats_per_vertex * 4)

def unpack_vertex(mode: ConversionMode, data: bytes, offset: int):
    return unpack_dynamic_vertex(data, offset) if mode == ConversionMode.DYNAMIC else unpack_static_vertex(data, offset)

def main():
    if len(sys.argv) != 4:
        print("Usage: ./binary_to_obj.py <dynamic/static> <input_file> <output_name>")
        sys.exit(1)
    mode = sys.argv[1]
    if mode != ConversionMode.DYNAMIC and mode != ConversionMode.STATIC:
        print("Mode must be either dynamic or static.")
        sys.exit(1)
    base64_input = ""
    with open(sys.argv[2], "r") as file_handle:
        base64_input = file_handle.read()
    obj_output_path = sys.argv[3] + ".obj"
    mtl_output_path = sys.argv[3] + ".mtl"

    data = base64.b64decode(base64_input)
    offset = 0
    vertices = []
    while offset < len(data):
        vertex, offset_increment = unpack_vertex(mode, data, offset)
        vertices.append(vertex)
        offset += offset_increment
    obj_buffer = f"o mesh\nmtllib {mtl_output_path}\n"
    mtl_buffer = ""
    position_indices = {}
    normal_indices = {}
    materials = {}
    # Write vertex positions
    counter = 0
    for vertex in vertices:
        if vertex.position not in position_indices:
            obj_buffer += vertex.position_line() + "\n"
            position_indices[vertex.position] = counter + 1
            counter += 1
    # Write normal data
    counter = 0
    for vertex in vertices:
        if vertex.normal not in normal_indices:
            obj_buffer += vertex.normal_line() + "\n"
            normal_indices[vertex.normal] = counter + 1
            counter += 1
    obj_buffer += "s 0\n"
    # Write material data
    if mode == ConversionMode.STATIC:
        counter = 0
        for vertex in vertices:
            if vertex.color not in materials:
                material_name = f"material_{counter}"
                materials[vertex.color] = material_name
                mtl_buffer += f"newmtl {material_name}\nKd {vertex.color[0]} {vertex.color[1]} {vertex.color[2]}\n"
                counter += 1
    # Group faces based on material used
    grouped_faces = {}
    for i in range(0, len(vertices), 3):
        v0, v1, v2 = vertices[i], vertices[i + 1], vertices[i + 2]
        # Assume that all vertices use the same material
        material_name = materials[v0.color] if mode == ConversionMode.STATIC else "empty"
        p0, p1, p2 = position_indices[v0.position], position_indices[v1.position], position_indices[v2.position]
        n0, n1, n2 = normal_indices[v0.normal], normal_indices[v1.normal], normal_indices[v2.normal]
        face_str = f"f {p0}//{n0} {p1}//{n1} {p2}//{n2}\n" # Faces are indexed from 1
        if material_name not in grouped_faces:
            grouped_faces[material_name] = []
        grouped_faces[material_name].append(face_str)
    for material, faces in grouped_faces.items():
        obj_buffer += f"usemtl {material}\n"
        for face in faces:
            obj_buffer += face

    # Write the OBJ to disk
    with open(obj_output_path, "w") as file_handle:
        file_handle.write(obj_buffer)

    # Write material to disk
    with open(mtl_output_path, "w") as file_handle:
        file_handle.write(mtl_buffer)

if __name__ == "__main__":
    main()