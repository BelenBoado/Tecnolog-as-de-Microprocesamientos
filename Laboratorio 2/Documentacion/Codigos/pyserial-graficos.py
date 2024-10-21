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
    line = ser.readline().decode('ascii', errors='ignore').strip()
    print(line)

    if line:
        data = line.split(',')

        if len(data) >= 3:
            try:

                if ':' in data[0] and ':' in data[1] and ':' in data[2]:
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
                    plt.pause(0.0001)
                else:
                    print("Formato de datos incorrecto, falta ':' en uno de los elementos")
            except ValueError:
                print("Error al convertir datos a entero, verifique el formato de los datos")
        else:
            print(f"Cantidad de datos insuficiente: {len(data)}. Datos recibidos: {data}")

