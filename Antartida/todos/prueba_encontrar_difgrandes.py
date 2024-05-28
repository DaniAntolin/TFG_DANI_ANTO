import pandas as pd

# Cargar el archivo CSV en un DataFrame
df = pd.read_csv('archivo_combinado3.csv', sep=';')

# Convertir la columna 'hora' a tipo datetime para facilitar los cálculos
df['hora'] = pd.to_datetime(df['hora'], format='%H:%M:%S')

# Calcular la diferencia entre cada fila y la fila anterior en la columna 'hora'
df['diferencia'] = df['hora'].diff()

# Calcular el valor absoluto de la diferencia
df['diferencia_abs'] = df['diferencia'].abs()

# Convertir la diferencia de tiempo a segundos
df['diferencia_segundos'] = df['diferencia_abs'].dt.total_seconds()

# Filtrar los valores de la columna 'diferencia_segundos' que sean mayores a 100
valores_suprimidos = df.loc[df['diferencia_segundos'] > 100, 'diferencia_segundos']

# Obtener los índices de los puntos donde la diferencia es mayor a 100 segundos
indices_mayor_100 = df.index[df['diferencia_segundos'] > 100]

# Crear una nueva columna para el número de punto correspondiente
df['num_punto'] = 0

# Asignar números de punto a las filas correspondientes
for i, idx in enumerate(indices_mayor_100):
    df.at[idx, 'num_punto'] = i + 1

# Guardar el DataFrame modificado en un nuevo archivo CSV
df.to_csv('archivo_combinado_modificado3.csv', sep=';', index=False)
