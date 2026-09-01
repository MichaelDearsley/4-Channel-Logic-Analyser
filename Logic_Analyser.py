import serial #Imports PySerial
import matplotlib.pyplot as plt #Imports plotting tools to form graph

arduino = serial.Serial("COM3", 115200, timeout=0) #Sets up Serial communications through COM3 at 115200 baud
fig, ax = plt.subplots() #Creates matplotlib figure and axis
plt.show(block=False) #Opens graph without stopping python

while True:

    samples = [] #List for 1000 samples

    while len(samples) < 1000:

        plt.pause(0.001) #Allows Matplotlib to process GUI events

        while arduino.in_waiting > 0: 

            line = arduino.readline().decode().strip() #Reads the incoming bytes, converts them to text and removes whitespace

            if line: #Check data is recieved 

                try:
                    value_of_byte = int(line, 2) #Attempt to convert data into number
                    samples.append(value_of_byte) #Stores number in array

                except ValueError: 
                    pass

            if len(samples) >= 1000: #Once 1000 samples has been captured break
                break


    print("Received", len(samples)) 

    channel_1 = [(i >> 0) & 1 for i in samples] #Extracts bit 0 from the byte which correlates to channel 1
    channel_2 = [(i >> 1) & 1 for i in samples] #Extracts bit 1 from the byte which correlates to channel 2
    channel_3 = [(i >> 2) & 1 for i in samples] #Extracts bit 2 from the byte which correlates to channel 3
    channel_4 = [(i >> 3) & 1 for i in samples] #Extracts bit 3 from the byte which correlates to channel 4

    decoded_message = "" #Start UART decoder

    i = 1 #Position for CH1 sample list

    while i < len(channel_1) - 100: #Keep searching while there is atleast another 100 samples left as 1 UART frame is 10 bits at 10 samples per bit is 100 samples

        if channel_1[i - 1] == 1 and channel_1[i] == 0: #UART starts HIGH then goes LOW so look for change

            data_bits = [] #List for UART data bits

            for bit in range(8): 

                sample_position = (i + 15 + round(bit * 10.4167)) #Sample in the middle of each UART bit, at 9600 baud, 1/9600 = 104us with the analyser at 100kHz hence 1/100kHz = 10us hence 104us/10us = 10.4 samples per UART bit

                if sample_position < len(channel_1):

                    data_bits.append(channel_1[sample_position]) #Stores value of sample position

            if len(data_bits) == 8: #Ensure all 8 bits collected

                value = 0 #Creates an empty value to reconstruct the byte
                for bit in range(8): #Reconstruct the byte

                    value |= (data_bits[bit] << bit) #Places each bit into correct position

                if 32 <= value <= 126: #Checks if number presents a normal printable ASCII character

                    decoded_message += chr(value) #Converts number to character

            i += 100 #Moves approximately one UART frame forward

        else:

            i += 1 #If no bit was found move on

    print("UART:", decoded_message)

    channel_2 = [i + 2 for i in channel_2] #Move CH2 up
    channel_3 = [i + 4 for i in channel_3] #Move CH3 up
    channel_4 = [i + 6 for i in channel_4] #Move CH4 up

    time_axis = range(1000) #Create x axis

    ax.clear() #Clear previous graph

    ax.step(time_axis, channel_1, label="Channel 1") #Draw CH1 as step function
    ax.step(time_axis, channel_2, label="Channel 2") #Draw CH2 as step function
    ax.step(time_axis, channel_3, label="Channel 3") #Draw CH3 as step function
    ax.step(time_axis, channel_4, label="Channel 4") #Draw CH4 as step function

    ax.set_yticks([0.5, 2.5, 4.5, 6.5],["CH1", "CH2", "CH3", "CH4"]) #Sets y axis labels
    ax.set_title("4 Channel Logic Analyser") #Names graph
    ax.set_xlabel("Time (samples)") #labels x axis
    ax.set_ylabel("Channels") #Labels vertical axis
    ax.grid(True) #Turns on grid
    ax.text(0.02,1.08,"UART: " + decoded_message,transform=ax.transAxes,fontsize=12,verticalalignment="top") #Adds UART text onto graph
    fig.canvas.draw_idle() #Redraw the graph
    fig.canvas.flush_events() #Allows GUI to process pending window events
    plt.pause(0.001) #Gives GUI more time
