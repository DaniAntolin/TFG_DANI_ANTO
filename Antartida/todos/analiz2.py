import pandas as pd
df = pd.read_csv('archivo_combinado_modificadoDTtot.csv', sep=';')
filas_99990 = df[df.iloc[:, 3] == 999.90]
print("Filas donde el valor en la tercera columna es 999.90:")
print(filas_99990)
valores_segunda_columna = filas_99990.iloc[:, 2]
print("Valores de la segunda columna para las filas que contienen el valor 999.90 en cualquier lugar:")
print(valores_segunda_columna)
filas_hum = df[df.iloc[:, 4] == -1.00]
print("Filas donde el valor hum es -1:")
print(filas_hum)
filas_veloc = df[df.iloc[:, 5] == -1.00]
print("Filas donde el valor velocidad es -1:")
print(filas_veloc)
filas_direct = df[df.iloc[:, 6] == -1]
print("Filas donde el valor direccion es -1:")
print(filas_direct)