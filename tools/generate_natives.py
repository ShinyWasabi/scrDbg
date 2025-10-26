import struct

INPUT_FILE = "natives.txt"
OUTPUT_FILE = "../resources/natives.bin"

def parse_line(line):
    line = line.strip()
    if not line or line.startswith("#"):
        return None
    try:
        parts = line.split(maxsplit=1)
        hash = int(parts[0], 16)
        name = parts[1].strip()
        return hash, name
    except Exception as e:
        print(f"Skipping invalid line: {line} ({e})")
        return None

def main():
    entries = []
    with open(INPUT_FILE, "r", encoding="utf-8") as f:
        for line in f:
            parsed = parse_line(line)
            if parsed:
                entries.append(parsed)

    with open(OUTPUT_FILE, "wb") as out:
        out.write(struct.pack("<I", len(entries)))
        for hash, name in entries:
            name_bytes = name.encode("utf-8")
            out.write(struct.pack("<QH", hash, len(name_bytes)))
            out.write(name_bytes)

    print(f"Wrote {len(entries)} entries to {OUTPUT_FILE}.")

if __name__ == "__main__":
    main()