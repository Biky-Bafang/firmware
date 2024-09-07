import os
import struct
import re

# Define paths
dir_paths = ["../data/flows/core", "../data/flows/custom"]

# Structs in Python
class Variable:
    def __init__(self, name, var_type, value, persist):
        self.name = name
        self.var_type = var_type
        self.value = value
        self.persist = persist

    def to_bytes(self):
        # Convert Variable to bytes: name, type, value, persist
        name_bytes = self.name.encode('utf-8')
        type_bytes = self.var_type.encode('utf-8')
        value_bytes = self.value.encode('utf-8')
        persist_byte = struct.pack('?', self.persist)

        # Length-prefixed string encoding
        return (
            struct.pack('I', len(name_bytes)) + name_bytes +
            struct.pack('I', len(type_bytes)) + type_bytes +
            struct.pack('I', len(value_bytes)) + value_bytes +
            persist_byte
        )

class FlowData:
    def __init__(self, file_id, trigger_type, trigger_data, lua_code, variables):
        self.id = file_id
        self.trigger_type = trigger_type
        self.trigger_data = trigger_data
        self.lua_code = lua_code
        self.variables = variables

    def to_bytes(self):
        # Convert FlowData to bytes
        id_bytes = self.id.encode('utf-8')
        trigger_type_bytes = self.trigger_type.encode('utf-8')
        lua_code_bytes = self.lua_code.encode('utf-8')

        trigger_data_bytes = bytes(self.trigger_data)  # Already a list of uint8

        # Length-prefixed string encoding
        return (
            struct.pack('I', len(id_bytes)) + id_bytes +
            struct.pack('I', len(trigger_type_bytes)) + trigger_type_bytes +
            struct.pack('I', len(trigger_data_bytes)) + trigger_data_bytes +
            struct.pack('I', len(lua_code_bytes)) + lua_code_bytes +
            struct.pack('I', len(self.variables)) + b''.join(var.to_bytes() for var in self.variables)
        )

# Helper function to convert string to bool
def str_to_bool(s):
    return s == '1'

# Function to parse Lua file and create FlowData
def parse_lua_file(file_path):
    with open(file_path, 'r') as file:
        lines = file.readlines()

    variables = []
    lua_code_lines = []
    trigger_type = ""
    trigger_data = []
    file_id = os.path.basename(file_path).replace(".lua", "")
    
    for line in lines:
        line = line.strip()
        if line.startswith("--"):
            # Parse metadata comments
            if "SERIALTRIGGER" in line:
                trigger_type, hex_data = line[2:].strip().split("-")
                trigger_data = [int(hex_val, 16) for hex_val in re.findall(r'0x[0-9a-fA-F]+', hex_data)]
            elif "var" in line:
                var_parts = line[2:].strip().split("-")
                var_name = var_parts[0]
                var_type = var_parts[1]
                var_value = var_parts[2]
                var_persist = str_to_bool(var_parts[3])
                variables.append(Variable(var_name, var_type, var_value, var_persist))
        else:
            # Non-comment lines are Lua code
            lua_code_lines.append(line)
    
    # Construct the lua code
    lua_code = "\n".join(lua_code_lines)
    
    # Create the FlowData struct
    flow_data = FlowData(file_id, trigger_type, trigger_data, lua_code, variables)
    
    return flow_data

# Function to serialize FlowData and dump into .bin file
def write_to_bin(flow_data, output_file):
    with open(output_file, 'wb') as f:
        f.write(flow_data.to_bytes())

# Process all .lua files in the directory and convert them to .bin
def process_directory(directory):
    for filename in os.listdir(directory):
        if filename.endswith(".lua"):
            lua_file_path = os.path.join(directory, filename)
            flow_data = parse_lua_file(lua_file_path)

            # Output .bin file
            bin_file_path = os.path.join(directory, filename.replace(".lua", ".bin"))
            write_to_bin(flow_data, bin_file_path)
            print(f"Converted {filename} to {bin_file_path}")

# Run the script
for dir_path in dir_paths:
	process_directory(dir_path)
