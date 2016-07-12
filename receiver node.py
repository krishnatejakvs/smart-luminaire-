import serial
import struct
from time import sleep,strftime
import datetime
import requests

readings=[0,2,4,6,8,10,12,14,16,18,20]
dimming=[0,70,100,120,140,155,170,185,205,225,255]
elecmfactor=3.86
mfactor=1.4
newpwm=0
lesser=0
greater=0
count=0
setpoint=300
index=0
lux=0
pwm=0
taskplaneill=1.0
pretaskplaneill=1.0

def read_serial(ser):
    buf = []
    sleep(0.3)
    buf.append(ser.read(200))
    #sleep(1)
    #buf.append(ser.read(ser.inWaiting()))
    return buf

def write_serial(data,ser):
    data=''.join(struct.pack('>B', x) for x in data)
    for x in data:
        ser.write(x)
    return
def initialise():
    #temp1=scan()
    ser = serial.Serial(port='COM4',stopbits=serial.STOPBITS_ONE,bytesize=serial.EIGHTBITS,parity=serial.PARITY_NONE,timeout=0,baudrate=9600,rtscts=False)
    ser.close()
    ser.open()
    return ser

def interiorRecalibration(lux, prelux,readings,setpoint):
    changescale=1.0+((lux-prelux)*1.0/prelux)
    print changescale
    old_readings = readings
    new_readings = map(lambda x: x*changescale, old_readings)
    readings = new_readings
    
    setpoint=setpoint*changescale
    print readings
    return new_readings

def illuminance(pwm, lux):
    global readings,index,elecmfactor,mfactor,dimming,taskplaneill
    for i in range(0,11):
        if(dimming[i]==pwm):
            index=i
    daylight=lux-60-readings[index]
    taskplaneill=(daylight*mfactor)*1.0+(readings[index]*elecmfactor)*1.0 +60*2.7
    return taskplaneill


def newPWM(error, pwm,ser):
    global index,elecmfactor,dimming,lesser,greater,newpwm
    print newPWM
    cmdinc=[0x7E ,0x00 ,0x13 ,0x10 ,0x00 ,0x00 ,0x13 ,0xA2 ,0x00 ,0x40 ,0x89 ,0xCC ,0xB7 ,0xFF ,0xFE ,0x00 ,0x00 ,0x99 ,0x03 ,0x49 ,0x4E ,0x43 ,0x7B]
    cmddec=[0x7E ,0x00 ,0x13 ,0x10 ,0x00 ,0x00 ,0x13 ,0xA2 ,0x00 ,0x40 ,0x89 ,0xCC ,0xB7 ,0xFF ,0xFE ,0x00 ,0x00 ,0x99 ,0x03 ,0x44 ,0x45 ,0x43 ,0x89]
    voltagemFactor= 0.088320488
    closedloopchange=error/elecmfactor
    newpwm = pwm +(closedloopchange/voltagemFactor)
    lesser=0
    greater=0
    if(newpwm<0):
        lesser=1
        greater=0
        newpwm=0
    elif(newpwm>255):
        greater=1
        lesser=0
        newpwm=255
    else:
        mindiff = 500
        mynewpwm = 0
        for i in range(len(dimming)):
            val = abs(dimming[i]-newpwm)
            if(val < mindiff):
                mynewpwm = dimming[i]
                mindiff = val
        newpwm = mynewpwm

    index1 = dimming.index(pwm)
    index2 = dimming.index(newpwm)
    iterations = index2-index1
    if (iterations > 0):
        for i in range(iterations):
            write_serial(cmdinc,ser)
    else:
        for i in range(iterations):
            write_serial(cmddec,ser)
    return newpwm

def voltageadjust():
    global pwm,lux,ser,taskplaneill,pretaskplaneill
    taskplaneill=illuminance(pwm, lux)
    newpwm = newPWM((pretaskplaneill-taskplaneill),pwm,ser)
    pwm=newpwm
    pretaskplaneill=taskplaneill
    return


def changeVoltageOnShadeChange(ser):
    global lux,pwm
    print "changeVoltageOnShadeChange"
    rx=[]
    rx=read_serial(ser)
    if(len(rx[0])>25):
        if(rx[0][15]=='\x99'):
            print "device1"
            print rx
            lux=int(rx[0][17])*1000+int(rx[0][18])*100+int(rx[0][19])*10+int(rx[0][20])*1
            pwm=int(rx[0][21])*1000+int(rx[0][22])*100+int(rx[0][23])*10+int(rx[0][24])*1
            relay=int(rx[0][25])
    return lux



def changeShadesPosition(ser):
    #cmd_up=[0x7E ,0x00 ,0x12 ,0x10 ,0x01 ,0x00 ,0x13 ,0xA2 ,0x00 ,0x40 ,0xA8 ,0x35 ,0x1A ,0xFF ,0xFE ,0x00 ,0x00 ,0x03 ,0x49 ,0x4E ,0x43 ,0x28] 
    #cmd_down=[0x7E ,0x00 ,0x12 ,0x10 ,0x01 ,0x00 ,0x13 ,0xA2 ,0x00 ,0x40 ,0xA8 ,0x35 ,0x1A ,0xFF ,0xFE ,0x00 ,0x00 ,0x03 ,0x44 ,0x45 ,0x43 ,0x36]
    global pwm,newpwm,lux,taskplaneill,setpoint,lesser,greater,count
    print "changeShadesPosition"
    taskplaneill=illuminance(newpwm, lux)
    if(greater != 1 and lesser != 1 and count > 0): # In case of voltage within range.
        while(count>0):
            #write_serial(cmd_down,ser)
            requests.get("http://10.2.32.136/ajax?frame=0")
            count -= 1
            sleep(7)
            lux=changeVoltageOnShadeChange(ser)
            taskplaneill=illuminance(pwm, lux)
            if(abs(taskplaneill-setpoint) > 10):
                voltageadjust()
            else:
                break
            if(lesser):
                break
            
    elif (greater): # In case of voltage beyond max.
        while(count>0):
            #write_serial(cmd_down,ser)
            requests.get("http://10.2.32.136/ajax?frame=0")
            count-=1
            sleep(7)
            lux=changeVoltageOnShadeChange(ser)
            taskplaneill=illuminance(pwm, lux)
            if(abs(taskplaneill-setpoint) > 10):
                voltageadjust()
            else:
                break
            if(lesser):
                break
            
    else: # In case of volatage below min.
        while(count < 20): # N is total length of the panel.
            #write_serial(cmd_up,ser)
            requests.get("http://10.2.32.136/ajax?frame=1")
            count+=1
            sleep(7)
            lux=changeVoltageOnShadeChange(ser)
            taskplaneill=illuminance(pwm, lux)
            if(abs(taskplaneill-setpoint) < 10):
                break
    
    return 
    
flag=1
tim1=0
previous_update = strftime("%M");
print readings
ser=initialise()
ser.flushInput()
cmd=[0x7E ,0x00 ,0x14 ,0x10 ,0x00 ,0x00 ,0x13 ,0xA2 ,0x00 ,0x40 ,0x89 ,0xCD ,0x04 ,0xFF ,0xFE ,0x00 ,0x00 ,0x99 ,0x04 ,0x53 ,0x54 ,0x41 ,0x54 ,0xCA]
with open('test10.csv', 'a') as fp:
    init=1
    pwm=255
    prepwm=255
    while 1:    
        rx=[]
        rx=read_serial(ser)
        if(len(rx[0])>25):
                if(rx[0][15]=='\x99'):
                    print "device1"
                    print rx
                    lux=int(rx[0][17])*1000+int(rx[0][18])*100+int(rx[0][19])*10+int(rx[0][20])*1
                    pwm=int(rx[0][21])*1000+int(rx[0][22])*100+int(rx[0][23])*10+int(rx[0][24])*1
                    relay=int(rx[0][25])
                    taskplaneill=illuminance(pwm, lux)
                    if(init==1):
                        prepwm=pwm
                        prelux=lux
                        taskplaneill=illuminance(pwm, lux)
                        pretaskplaneill=taskplaneill
                        init=0
                    print lux," ",pwm," ",relay," ",prelux," "              
                    if(abs(prelux-lux)>10 and relay):
                        print "diff>10"
                        ser.flushInput()
                        write_serial(cmd,ser)
                        flag=1
                        while (flag):                        
                            rx=read_serial(ser)
                            print rx
                            if(len(rx[0])>16):
                                if(rx[0][15]=='\x98'):
                                    print "device 2"
                                    flag=0
                                    print rx
                                    if(rx[0][17]=='N'):
                                        if(abs(prepwm-pwm)==0):
                                            output_readings = interiorRecalibration(lux, prelux,readings,setpoint)
                                            readings = output_readings
                                             #recalibration
                                        else:
                                            voltageadjust() #to compensate relay effect after occupancy
                                    else:
                                        r=requests.get("http://10.2.32.136/get_count") #check if shades moved
                                        count1=int(r.content)
                                        if(count!=count1):
                                            count=count1
                                            taskplaneill=illuminance(pwm, lux)
                                            setpoint=taskplaneill
                                        else:
                                            voltageadjust() #to compensate effect of daylight
                                            #use time condition here for shades movement and write a funtion to bring back shades to zero after 6pm
                                            a = strftime("%H")
                                            if(int(a) > 6 and int(a) < 18):
                                                times=1
                                                current_time = strftime("%M")
                                                if((taskplaneill<(1.10*setpoint) or taskplaneill>(0.85*setpoint)) and (abs(current_time - previous_update) >= 2)):
                                                    changeShadesPosition(ser)
                                                    previous_update = strftime("%M")
                                            else:
                                                if(times==1):
                                                    for i in range(1,count):
                                                        requests.get("http://10.2.32.136/ajax?frame=0")
                                                        sleep(7)
                                                    times=0 
                            else:
                                write_serial(cmd,ser)
                for i in range(0,11):
                    if(dimming[i]==pwm):
                        index=i
                r=requests.get("http://10.2.32.136/get_count") #check if shades moved
                count1=int(r.content)
                #print taskplaneill," ",daylight," ",readings[index]," ",readings[10]
                print taskplaneill
                pretaskplaneill = taskplaneill
                prelux=lux
                prepwm=pwm
                fp.write("%s \t %d \t %d \t %.1f \t %d \t %.1f \t %d \n %.1f \n" %(datetime.datetime.now().time(),lux,index,taskplaneill,count,readings[10],relay,setpoint))
        else:
                continue
