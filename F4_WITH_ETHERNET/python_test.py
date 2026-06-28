import pyvisa
import time
rm = pyvisa.ResourceManager()
#IP 192.168.0.51 PORT 7
VISA_STRING = 'TCPIP0::192.168.0.51::7::SOCKET'
#VISA_STRING = 'ASRL8::INSTR' #Serial port for testing with USB to UART adapter

instrument = rm.open_resource(VISA_STRING)
instrument: pyvisa.resources.tcpip.TCPIPInstrument
instrument.write_termination = '\n'
instrument.read_termination = '\n'
instrument.timeout = 7000 #Set timeout to 2 seconds

# Example command to read the control register of the I2C expander
print(instrument.query('*IDN?'))

def initTOF(channel, ext_pin):
    ################### INIT OF SENSOR TOF 0 ######################
    instrument.query(f'EXT_SET_EN_PIN 1 1')#ENABLE LOGIC LEVEL SHIFTER
    instrument.query(f'SET_EXT_PIN {ext_pin} 0')#HOLDING TOF IN RESET
    instrument.query(f'I2C_EXPANDER INIT')
    instrument.query(f'I2C_EXPANDER SELECT_CHANNEL {channel}')
    instrument.query(f'SET_EXT_PIN {ext_pin} 1')#RELEASING TOF FROM RESET

    instrument.query(f'TOF {channel} SET_I2C_SPEED 100000')
    instrument.query(f'TOF {channel} INIT')
    instrument.query(f'TOF {channel} GET_BOOT_STATE')
    instrument.query(f'TOF {channel} SET_MODE SHORT')
    instrument.query(f'TOF {channel} SET_INTER_MEASUREMENT_DELAY 20')
    instrument.query(f'TOF {channel} SET_TIMING_BUDGET 20')
    #instrument.query(f'TOF {channel} SET_100HZ_MODE')
    #print(instrument.query(f'TOF {channel} SET_ROI 16 16'))

    #SETUP INTERRUPT ON LIMITS
    #TOF [0-7] SET_DISTANCE_TRESHOLD [MIN_MM] [MAX_MM] [0:bellow|1:above|2:out|3:in] [IntOnNoTarget]

    print(instrument.query(f'TOF {channel} TOF 7 SET_DISTANCE_TRESHOLD 80 500 0 '))
    print(instrument.query(f'TOF {channel} GET_DISTANCE_TRESHOLD'))

    #READBACK TRESHOLDS

    last_resp = instrument.query(f'TOF {channel} MEASURE_DISTANCE 200')
    if "OK" in last_resp:
        print(f'TOF {channel} distance measurement: {last_resp}')
    else:
        print("ERROR", last_resp)

initTOF(0, 5) #Init TOF on channel 0 with XSHUT pin held low (reset)
initTOF(7, 1) #Init TOF on channel 7 with
#
#print(instrument.query('TOF 0 SET_FLAG AUTOMATED_MEASUREMENT 0'))
#print(instrument.query('TOF 0 SET_FLAG AUTOMATED_MEASUREMENT 1'))



if 10:
    #print(instrument.query('TOF 0 DO_LIMIT_CALIBRATION 5000 30'))
    #print(instrument.query('TOF 7 DO_LIMIT_CALIBRATION 5000 30'))

    print(instrument.query('TOF 0 GET_FLAG PERSISTENT_OUT_OF_LIMIT'))
    print(instrument.query('TOF 0 GET_FLAG PERSISTENT_IN_LIMIT'))
    print(instrument.query('TOF 0 GET_FLAG IS_INSIDE_LIMITS'))
    print(instrument.query('TOF 0 GET_FLAG AUTOMATED_MEASUREMENT_IS_RUNNING'))
    print(instrument.query('TOF 0 GET_FLAG LAST_ERROR'))

    ##CLEAR ALL ERROR FLAGS
    print(instrument.query('TOF 0 SET_FLAG LAST_ERROR 0'))
    print(instrument.query('TOF 7 SET_FLAG LAST_ERROR 0'))


    #SET AUTOMATED MEASUREMENT ON TOF 0
    print(instrument.query('TOF 0 SET_FLAG AUTOMATED_MEASUREMENT 0'))
    print(instrument.query('TOF 0 GET_FLAG AUTOMATED_MEASUREMENT_IS_RUNNING'))


    print(instrument.query('TOF 7 SET_FLAG AUTOMATED_MEASUREMENT 0'))
    print(instrument.query('TOF 7 GET_FLAG AUTOMATED_MEASUREMENT_IS_RUNNING'))

    #read error flags
    time.sleep(1)
    print(instrument.query('TOF 0 GET_FLAG LAST_ERROR'))
    print(instrument.query('TOF 7 GET_FLAG LAST_ERROR'))


    print(instrument.query('TOF 0 SET_FLAG ALL_PERSISTENT_FLAGS 0'))
    print(instrument.query('TOF 7 SET_FLAG ALL_PERSISTENT_FLAGS 0'))
    print(instrument.query('*IDN?'))



#CLEAR BUFFER
while True:
    try:
        instrument.timeout = 20
        print(instrument.read())
    except Exception as e:
        break

instrument.timeout = 2000

timeout = time.time()+10
min_value = 65535
max_value = 0
while time.time() < timeout:
    #FETCHING MIN MAX VALUES OF SIGNAL STRENGT
    result = instrument.query('TOF 7 RAW_I2C_READ_WORD 152').split(":")[-1]
    result_value = int(result,16)
    if result_value < min_value:
        min_value = result_value
    if result_value > max_value:
        max_value = result_value

#UPDATE LIMITS WIT 10% OFFSET
min_value = min_value * 0.9
max_value = max_value * 1.1
print("MIN VALUE:", min_value)
print("MAX VALUE:", max_value)

if 1:
    while True:
        result = instrument.query('TOF 7 RAW_I2C_READ_WORD 152').split(":")[-1]
        result_value = int(result,16)
        #print in green if value is between limits, print in red if value is out of limits
        if result_value < min_value or result_value > max_value:
            print("\033[91m" + str(result_value) + "\033[0m", end=' ')
        else:
            print("\033[92m" + str(result_value) + "\033[0m", end=' ')
        
        print(" ")
        time.sleep(0.002)


if 0:
    while True:
        time.sleep(0.1)
        response = instrument.query('TOF 7 CHECK_INTERRUPT')
        if response == "OK;TOF 7 CHECK_INTERRUPT active:3":
            print(time.time(),"INTERRUPT DETECTED ON TOF 7")
            instrument.query('TOF 7 CLEAR_INTERRUPT')


while True:
    #switchTOF(0) #Init TOF on channel 0 with XSHUT pin held low (reset)
#    res = instrument.query("TOF 0 GET_FLAG AUTOMATED_MEASUREMENT_IS_RUNNING")
#    if res == "OK;TOF 0 FLAG AUTOMATED_MEASUREMENT_IS_RUNNING:1":
#        in_limit_flag = instrument.query('TOF 0 GET_FLAG IS_INSIDE_LIMITS')
#        in_limit_flag_value = int(in_limit_flag.split(":")[-1])
#        persistent_out_of_limit_flag = instrument.query('TOF 0 GET_FLAG PERSISTENT_OUT_OF_LIMIT')
#        distance = instrument.query('TOF 0 GET_LAST_AUTOMATED_MEASUREMENT')
#        persistent_out_of_limit_flag_value = int(persistent_out_of_limit_flag.split(":")[-1])
#        persistent_in_limit_flag = instrument.query('TOF 0 GET_FLAG PERSISTENT_IN_LIMIT')
#        persistent_in_limit_flag_value = int(persistent_in_limit_flag.split(":")[-1])
#
#        if in_limit_flag_value == 1:
#            #Print in green
#            print("\033[92m" + distance + "\033[0m", end=' ')
#        #print in orange
#        else:
#            print("\033[93m" + distance + "\033[0m", end=' ')
#
#        #ADD INFO ABOUT PERSISTENT FLAGS (IF 1 PRINT THEM GREEN, IF 0 PRINT THEM RED)
#        if persistent_out_of_limit_flag_value == 1:
#            print("\033[91m" + "PERSISTENT OUT OF LIMIT DETECTED" + "\033[0m", end=' ')
#        else:
#            print("\033[92m" + "NO PERSISTENT OUT OF LIMIT" + "\033[0m", end=' ')
#        if persistent_in_limit_flag_value == 1:
#            print("\033[92m" + "PERSISTENT IN LIMIT DETECTED" + "\033[0m", end=' ')
#        else:
#            print("\033[91m" + "NO PERSISTENT IN LIMIT" + "\033[0m", end=' ')
#        print("")
#
#        
#            
#    else:
#        error=  instrument.query('TOF 0 GET_FLAG LAST_ERROR')
#        print(instrument.query('TOF 0 GET_FLAG LAST_ERROR'))
#        #Print in red
#        print("\033[91m" +"WTF???" + error + "\033[0m")
#
    res = instrument.query("TOF 7 GET_FLAG AUTOMATED_MEASUREMENT_IS_RUNNING")
    if res == "OK;TOF 7 FLAG AUTOMATED_MEASUREMENT_IS_RUNNING:1":
        in_limit_flag = instrument.query('TOF 7 GET_FLAG IS_INSIDE_LIMITS')
        in_limit_flag_value = int(in_limit_flag.split(":")[-1])
        distance = instrument.query('TOF 7 GET_LAST_AUTOMATED_MEASUREMENT')
        persistent_out_of_limit_flag = instrument.query('TOF 7 GET_FLAG PERSISTENT_OUT_OF_LIMIT')
        persistent_out_of_limit_flag_value = int(persistent_out_of_limit_flag.split(":")[-1])
        persistent_in_limit_flag = instrument.query('TOF 7 GET_FLAG PERSISTENT_IN_LIMIT')
        persistent_in_limit_flag_value = int(persistent_in_limit_flag.split(":")[-1])
        if in_limit_flag_value == 1:
            #Print in green
            print("\033[92m" + distance + "\033[0m", end=' ')
        #print in orange
        else:
            print("\033[93m" + distance + "\033[0m", end=' ')

        #ADD INFO ABOUT PERSISTENT FLAGS (IF 1 PRINT THEM GREEN, IF 0 PRINT THEM RED)
        if persistent_out_of_limit_flag_value == 1:
            print("\033[91m" + "PERSISTENT OUT OF LIMIT DETECTED" + "\033[0m", end=' ')
        else:
            print("\033[92m" + "NO PERSISTENT OUT OF LIMIT" + "\033[0m", end=' ')
        if persistent_in_limit_flag_value == 1:
            print("\033[92m" + "PERSISTENT IN LIMIT DETECTED" + "\033[0m", end=' ')
        else:
            print("\033[91m" + "NO PERSISTENT IN LIMIT" + "\033[0m", end=' ')
            
    else:
        error=  instrument.query('TOF 7 GET_FLAG LAST_ERROR')
        #Print in red
        print("\033[91m" + error + "\033[0m")

    print(" ")