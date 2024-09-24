import serial
import time

def speed_test_serial(com_port, baud_rate):
    try:
        # Open the serial port
        ser = serial.Serial(com_port, baud_rate, timeout=1)
        print(f"Opened {com_port} at {baud_rate} baud.")
        
        # Data to send
        data_to_send = bytes([0x11, 0x52])
        
        # Record the time before sending data
        
        # Send the data
        start_time = time.time()
        ser.write(data_to_send)
        print(f"Sent: {['0x{:02X}'.format(b) for b in data_to_send]}")
        
        # Wait for a response (blocking until a byte is received or timeout)
        response = ser.read()  # Adjust read size if you expect a multi-byte response
        end_time = time.time()
        # Record the time after receiving response

        if response:
            # Print the received response in hexadecimal format
            print(f"Received: {['0x{:02X}'.format(b) for b in response]}")
            elapsed_time = (end_time - start_time) * 1000  # Convert to milliseconds
            print(f"Elapsed time: {elapsed_time:.2f} ms")
        else:
            print("No response received within timeout period.")
        
        # Close the serial port
        ser.close()
        
    except serial.SerialException as e:
        print(f"Error opening or communicating with {com_port}: {e}")

speed_test_serial("COM9", 1200)
