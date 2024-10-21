# -*- coding: utf-8 -*-
"""
Created on Thu Oct 17 17:34:31 2024

@author: UTEC-5695
"""

import serial
import matplotlib.pyplot as plt

ser = serial.Serial('COM12', 9600) 

pot1_values = []
pot2_values = []
pwm_values = []

plt.ion()
fig, ax = plt.subplots()

while True:
    line = ser.readline().decode().strip()
    print(line)
    
    # Separamos los datos segun lleguen en formato "Pot1: X, Pot2: Y, PWM: Z, Dir: Horario/Antihorario"
    data = line.split(',')
    pot1 = int(data[0].split(':')[1].strip())
    pot2 = int(data[1].split(':')[1].strip())
    pwm = int(data[2].split(':')[1].strip())
    
    pot1_values.append(pot1)
    pot2_values.append(pot2)
    pwm_values.append(pwm)
    
    ax.clear()
    ax.plot(pot1_values, label="Pot1 (Referencia)")
    ax.plot(pot2_values, label="Pot2 (Motor)")
    ax.plot(pwm_values, label="PWM")
    ax.legend()
    plt.draw()
    plt.pause(0.1)

