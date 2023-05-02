import json
import os
def truncate():
    input_file = "../../Downloads/dblp.v12/dblp.v12.json"
    output_file = "output.json"

    # Open the input and output files

    if os.path.isfile(input_file):
        file_size = os.path.getsize(input_file)
        if file_size > 500000:
            with open(input_file, 'r+') as f:
                f.seek(500000)  # Move the file pointer to the 500kb mark
                f.truncate()  # Truncate the file from the current position
            print("File has been truncated to 500kb")
        else:
            print("File is already smaller than 500kb")
    else:
        print("File does not exist")


if __name__ == '__main__':
    truncate()

