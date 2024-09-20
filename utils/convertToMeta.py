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
        name_bytes = self.name.encode('utf-8')
        type_bytes = self.var_type.encode('utf-8')
        value_bytes = self.value.encode('utf-8')
        persist_byte = struct.pack('?', self.persist)

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
        id_bytes = self.id.encode('utf-8')
        trigger_type_bytes = self.trigger_type.encode('utf-8')
        lua_code_bytes = self.lua_code.encode('utf-8')
        trigger_data_bytes = bytes(self.trigger_data)

        return (
            struct.pack('I', len(id_bytes)) + id_bytes +
            struct.pack('I', len(trigger_type_bytes)) + trigger_type_bytes +
            struct.pack('I', len(trigger_data_bytes)) + trigger_data_bytes +
            struct.pack('I', len(lua_code_bytes)) + lua_code_bytes +
            struct.pack('I', len(self.variables)) + b''.join(var.to_bytes() for var in self.variables)
        )

# Utility to convert string to bool
def str_to_bool(s):
    return s == '1'

# Function to interactively input a variable
def input_variable():
    print("\n=== Create a New Variable ===")
    name = input("Variable Name: ")
    var_type = input("Variable Type (integer/string/boolean): ")
    value = input("Variable Value: ")
    persist = input("Persist across reboots? (1 for Yes, 0 for No): ")
    return Variable(name, var_type, value, str_to_bool(persist))

# Function to display and allow editing of variables
def edit_variables(variables):
    while True:
        print("\n=== Variables Configuration ===")
        for idx, var in enumerate(variables):
            print(f"{idx + 1}. Name: {var.name}, Type: {var.var_type}, Value: {var.value}, Persist: {var.persist}")
        print("\n(N)ew Variable, (E)dit Existing, (D)elete, (Q)uit")
        choice = input("Choose an option: ").lower()

        if choice == 'n':
            variables.append(input_variable())
        elif choice == 'e':
            var_idx = int(input("Variable number to edit: ")) - 1
            variables[var_idx] = input_variable()
        elif choice == 'd':
            var_idx = int(input("Variable number to delete: ")) - 1
            del variables[var_idx]
        elif choice == 'q':
            break
        else:
            print("Invalid choice, please try again.")

# Function to interactively input trigger type and data
def input_trigger():
    print("\n=== Configure Trigger ===")
    trigger_type = input("Choose Trigger Type (trigger/repeat): ").lower()

    if trigger_type == "trigger":
        trigger_data = input("Enter trigger hex values (comma-separated, e.g., 0x11, 0x52): ")
        trigger_data = [int(hex_val.strip(), 16) for hex_val in trigger_data.split(",")]
    elif trigger_type == "repeat":
        repeat_interval = int(input("Enter repeat interval in seconds: "))
        trigger_data = [repeat_interval]
    else:
        print("Invalid trigger type, setting default (trigger).")
        trigger_type = "trigger"
        trigger_data = [0x00]
    
    return trigger_type, trigger_data

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
            lua_code_lines.append(line)
    
    lua_code = "\n".join(lua_code_lines)
    flow_data = FlowData(file_id, trigger_type, trigger_data, lua_code, variables)
    
    return flow_data

# Function to serialize FlowData and write to .bin file
def write_to_bin(flow_data, output_file):
    with open(output_file, 'wb') as f:
        f.write(flow_data.to_bytes())

# Process Lua file and configure flow data interactively
def process_file(file_path):
    print(f"\nProcessing {file_path}...")

    flow_data = parse_lua_file(file_path)

    # Trigger configuration
    print("\nCurrent Trigger Type: ", flow_data.trigger_type)
    trigger_type, trigger_data = input_trigger()

    # Variable configuration
    edit_variables(flow_data.variables)

    flow_data.trigger_type = trigger_type
    flow_data.trigger_data = trigger_data

    bin_file_path = file_path.replace(".lua", ".bin")
    write_to_bin(flow_data, bin_file_path)
    print(f"Converted {file_path} to {bin_file_path}")

# Process all Lua files in directories
def process_directory(directory):
    for filename in os.listdir(directory):
        if filename.endswith(".lua"):
            lua_file_path = os.path.join(directory, filename)
            process_file(lua_file_path)

# Run the script
for dir_path in dir_paths:
    process_directory(dir_path)
