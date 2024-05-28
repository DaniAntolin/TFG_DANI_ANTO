import pandas as pd

# Cargar el archivo CSV en un DataFrame
df = pd.read_csv('archivo_combinado_modificado_DT2.csv', sep=';')

# Filtra el DataFrame para encontrar las filas donde el valor en la tercera columna sea 999.90
filas_99990 = df[df.iloc[:, 2] == 0]
# Imprime las filas que cumplen con el criterio
print("Filas donde el valor en la tercera columna es 999.90:")
print(filas_99990)
# Obtiene el valor de la segunda columna para las filas seleccionadas
valores_segunda_columna = filas_99990.iloc[:, 2]
# Imprime los valores de la segunda columna para las filas seleccionadas
print("Valores de la segunda columna para las filas que contienen el valor 999.90 en cualquier lugar:")
print(valores_segunda_columna)

# Filtra el DataFrame para encontrar las filas donde el valor en la tercera columna sea 999.90
filas_hum = df[df.iloc[:, 3] == 0]
# Imprime las filas que cumplen con el criterio
print("Filas donde el valor hum es -1:")
print(filas_hum)

# Filtra el DataFrame para encontrar las filas donde el valor en la tercera columna sea 999.90
filas_veloc = df[df.iloc[:, 4] == 0]
# Imprime las filas que cumplen con el criterio
print("Filas donde el valor velocidad es -1:")
print(filas_veloc)

# Filtra el DataFrame para encontrar las filas donde el valor en la tercera columna sea 999.90
filas_direct = df[df.iloc[:, 5] == 0]
# Imprime las filas que cumplen con el criterio
print("Filas donde el valor direccion es -1:")
print(filas_direct)
