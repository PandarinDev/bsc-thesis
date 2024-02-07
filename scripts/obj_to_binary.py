#!/usr/bin/env python3

import sys
import struct
import base64

# Converts OBJ files to binary representation that is expected by the program.
def main():
    args = sys.argv[1:]
    num_args = len(args)
    if num_args == 0:
        print("Usage: convert_obj.py <obj1> [<obj2> <...>]")
        sys.exit(1)
    for arg_index in range(num_args):
        file_path = args[arg_index]
        print(f"Trying to convert '{file_path}'.")
        binary_data = bytearray()
        # Read the data from the input OBJ file and store it in the binary buffer
        with open(file_path, "r") as file_handle:
            lines = file_handle.readlines()
            vertices = []
            normals = []
            for raw_line in lines:
                line = raw_line.strip()
                # Skip comment lines
                if line.startswith("#"):
                    continue
                # Vertex
                if line.startswith("v "):
                    data = line.split(" ")
                    vertices.append(tuple(float(data[i]) for i in range(1, 4)))
                # Normal
                elif line.startswith("vn "):
                    data = line.split(" ")
                    normals.append(tuple(float(data[i]) for i in range(1, 4)))
                # Face
                elif line.startswith("f "):
                    # Currently the script assumes that there are no texture coordinates
                    data = line.split(" ")
                    for i in range(1, 4):
                        parts = data[i].split("//")
                        vertex_index = int(parts[0]) - 1
                        normal_index = int(parts[1]) - 1
                        vertex = vertices[vertex_index]
                        normal = normals[normal_index]
                        binary_data.extend(struct.pack("ffffff", vertex[0], vertex[1], vertex[2], normal[0], normal[1], normal[2]))
        # Write the result as a base64 encoded string to stdout
        print(base64.b64encode(binary_data))


if __name__ == "__main__":
    main()