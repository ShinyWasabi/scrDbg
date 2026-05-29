import struct
import re

NATIVE_TYPE = {
    "NONE": 0,
    "INT": 1,
    "BOOL": 2,
    "FLOAT": 3,
    "STRING": 4,
    "REFERENCE": 5
}

# 0xHASH NAME <arg_types> <return_types>
LINE_REGEX = re.compile(
    r'^(0x[0-9A-F]+)\s+(\S+)\s+<([^>]*)>\s+<([^>]*)>'
)

def parse_types(type_str):
    types = []
    type_str = type_str.strip()
    if not type_str:
        return types

    for t in type_str.split(","):
        t = t.strip().upper()
        if t:
            types.append(NATIVE_TYPE.get(t, 0))
    return types


def parse_line(line):
    line = line.strip()
    if not line or line.startswith("#"):
        return None

    match = LINE_REGEX.match(line)
    if not match:
        print(f"Skipping invalid line: {line}")
        return None

    native_hash = int(match.group(1), 16)
    native_name = match.group(2)
    arg_types = parse_types(match.group(3))
    ret_types = parse_types(match.group(4))

    return native_hash, native_name, arg_types, ret_types


def build_bin(input_file, output_file):
    entries = []

    with open(input_file, "r", encoding="utf-8") as f:
        for line in f:
            parsed = parse_line(line)
            if parsed:
                entries.append(parsed)

    with open(output_file, "wb") as out:
        out.write(struct.pack("<I", len(entries))) # native count

        for native_hash, name, arg_types, ret_types in entries:
            name_bytes = name.encode("utf-8")
            out.write(struct.pack("<QH", native_hash, len(name_bytes))) # hash (uint64), name length (uint16)
            out.write(name_bytes) # name bytes

            out.write(struct.pack("<H", len(arg_types))) # argument count (uint16)
            for a in arg_types:
                out.write(struct.pack("B", a)) # argument type (uint8)

            out.write(struct.pack("<H", len(ret_types))) # return count (uint16)
            for r in ret_types:
                out.write(struct.pack("B", r)) # return type (uint8)

    print(f"Wrote {len(entries)} natives to {output_file}.")


def main():
    build_bin("natives-gta5.txt", "../resources/natives-gta5.bin")
    build_bin("natives-gta4.txt", "../resources/natives-gta4.bin")


if __name__ == "__main__":
    main()