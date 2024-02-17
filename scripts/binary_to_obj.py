#!/usr/bin/env python3

import sys
import struct
import base64
from dataclasses import dataclass
from typing import Tuple

@dataclass(frozen=True)
class Vertex:
    position: Tuple[float, float, float]
    normal: Tuple[float, float, float]
    color: Tuple[float, float, float]

    def position_line(self) -> str:
        return f"v {self.position[0]} {self.position[1]} {self.position[2]}"

    def normal_line(self) -> str:
        return f"vn {self.normal[0]} {self.normal[1]} {self.normal[2]}"

def main():
    if len(sys.argv) != 3:
        print("Usage: ./binary_to_obj.py <base64> <output_name>")
        sys.exit(1)
    base64_input = sys.argv[1]
    obj_output_path = sys.argv[2] + ".obj"
    mtl_output_path = sys.argv[2] + ".mtl"

    data = base64.b64decode(base64_input)
    floats_per_vertex = 9
    offset = 0
    vertices = []
    while offset < len(data):
        vertex_data = struct.unpack_from("f" * floats_per_vertex, data, offset)
        vertices.append(Vertex(
            (vertex_data[0], vertex_data[1], vertex_data[2]),
            (vertex_data[3], vertex_data[4], vertex_data[5]),
            (vertex_data[6], vertex_data[7], vertex_data[8])))
        offset += floats_per_vertex * 4
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
        material_name = materials[v0.color]
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