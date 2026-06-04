import sys
import struct

# Structure of the initrd header:
# uint32_t nfiles
# Array of file headers:
#   uint32_t magic (0xBF)
#   char name[64]
#   uint32_t offset
#   uint32_t length

def main():
    if len(sys.argv) < 3:
        print("Usage: python mkinitrd.py output_file input_file1 [input_file2 ...]")
        return

    output_filename = sys.argv[1]
    input_filenames = sys.argv[2:]

    nfiles = len(input_filenames)
    # nfiles (4) + nfiles * (4 + 64 + 4 + 4)
    header_size = 4 + nfiles * 76
    
    current_offset = header_size
    file_headers = []
    file_contents = []

    for filename in input_filenames:
        with open(filename, 'rb') as f:
            content = f.read()
            length = len(content)
            # Use only the filename part if a path is given
            name = filename.split('/')[-1]
            if len(name) > 63:
                name = name[:63]
            
            # magic, name, offset, length
            file_header = struct.pack('<I64sII', 0xBF, name.encode('ascii'), current_offset, length)
            file_headers.append(file_header)
            file_contents.append(content)
            current_offset += length

    with open(output_filename, 'wb') as f:
        f.write(struct.pack('<I', nfiles))
        for header in file_headers:
            f.write(header)
        for content in file_contents:
            f.write(content)

    print(f"Created {output_filename} with {nfiles} files.")

if __name__ == "__main__":
    main()
