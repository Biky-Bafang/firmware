import os
import json
import struct

def write_string(file, string):
    length = len(string)
    file.write(struct.pack('<H', length))  # Write the string length (2 bytes, little-endian)
    file.write(string.encode('utf-8'))     # Write the actual string

def dump_flow_data_to_binary(flow, lua_code, output_file):
    with open(output_file, 'wb') as file:
        # Write flow fields
        write_string(file, flow['name'])
        write_string(file, flow['id'])
        write_string(file, flow['trigger_type'])
        write_string(file, flow['trigger_data'])
        write_string(file, flow['trigger_device'])
        write_string(file, lua_code)
        
        # Write the variables
        num_variables = len(flow['variables'])
        file.write(struct.pack('<H', num_variables))  # Write number of variables (2 bytes)

        for var in flow['variables']:
            write_string(file, var['name'])
            write_string(file, var['type'])
            write_string(file, var['value'])

            # Write persist flag as 1 byte
            persist_byte = 1 if var['persist'] else 0
            file.write(struct.pack('B', persist_byte))

        # Write coreFlow flag as 1 byte
        core_flow_byte = 1 if flow['coreFlow'] else 0
        file.write(struct.pack('B', core_flow_byte))

        print(f"Flow data written to {output_file}")

def convert_json_and_lua_to_bin(flows_dir='./flows', output_dir='./output'):
    # Ensure the output directory exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    for filename in os.listdir(flows_dir):
        if filename.endswith('.json'):
            json_filename = os.path.join(flows_dir, filename)
            lua_filename = os.path.join(flows_dir, filename.replace('.json', '.lua'))
            bin_filename = os.path.join(output_dir, filename.replace('.json', '.bin'))

            if not os.path.exists(lua_filename):
                print(f"Lua file {lua_filename} not found, skipping.")
                continue

            # Read JSON data
            with open(json_filename, 'r') as json_file:
                flow_data = json.load(json_file)

            # Read Lua code
            with open(lua_filename, 'r') as lua_file:
                lua_code = lua_file.read()

            # Write to binary file in output directory
            dump_flow_data_to_binary(flow_data, lua_code, bin_filename)

# Run the conversion
convert_json_and_lua_to_bin('./flows', '../data/flows')
