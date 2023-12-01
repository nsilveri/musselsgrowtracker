import tkinter as tk
from tkinter import ttk, messagebox
import serial.tools.list_ports

def get_available_ports():
    return [port.device for port in serial.tools.list_ports.comports()]

def send_command(command):
    global ser
    if ser is not None:
        try:
            ser.write(command.encode('utf-8'))
            response = ser.readline().decode('utf-8')
            messagebox.showinfo("Response", response)
        except Exception as e:
            messagebox.showerror("Error", f"Failed to send '{command}' command: {e}")
    else:
        messagebox.showerror("Error", "Please select a serial port first.")

def button_click(command):
    send_command(command)

def on_port_selected(event):
    global ser
    ser = serial.Serial(selected_port.get(), 9600, timeout=1)
    ser.port = selected_port.get()

def on_closing():
    if ser is not None:
        ser.close()
    root.destroy()

root = tk.Tk()
root.title("Serial Command Interface")

# Creazione del frame per label e menu a tendina
serial_frame = tk.Frame(root)
serial_frame.pack(side=tk.TOP, pady=5)

serial_label = tk.Label(serial_frame, text="Serial port:")
serial_label.pack(side=tk.LEFT, padx=10, pady=5)

selected_port = tk.StringVar(value=get_available_ports()[0] if get_available_ports() else "")
ser = None

ports_menu = ttk.Combobox(serial_frame, textvariable=selected_port, values=get_available_ports())
ports_menu.pack(side=tk.LEFT, padx=10, pady=5)
ports_menu.bind("<<ComboboxSelected>>", on_port_selected)
ports_menu.set(selected_port.get())

commands = [
    ('Trigger GNSS', 's'), ('Read GPS Data', 'g'), ('Enable/Disable Debug', 'd'), ('Set Log Level 1', '1'),
    ('Set Log Level 2', '2'), ('Read GPS Time', 'o'), ('Init BMA400', 'a'), ('I2C Bus Scanning', 'i'),
    ('Enable/Disable Mosfet', 'm'), ('Read Load Cell Output', 'c'), ('Set Load Cell Offset', 'x'),
    ('Calibrate Load Cell', 'b'), ('Set Load Cell Scale', 'z'), ('Read Internal Memory Info', 'f'),
    ('Deep Sleep Test', 'p'), ('Turn On/Off Blue LED', 'l'), ('Get VDDA', 'v'), ('Get Temperature', 't')
]

# Organizza i pulsanti in quattro pulsanti per riga
for row in range(0, len(commands), 4):
    frame = tk.Frame(root)
    frame.pack(side=tk.TOP, pady=5)

    for label, command in commands[row:row + 4]:
        btn = tk.Button(frame, text=label, command=lambda c=command: button_click(c))
        btn.pack(side=tk.LEFT, padx=5)

root.protocol("WM_DELETE_WINDOW", on_closing)
root.mainloop()
