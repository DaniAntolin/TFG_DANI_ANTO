import pandas as pd

# Leer los tres archivos CSV en DataFrames separados
df1 = pd.read_csv('E1_P1.csv', sep=';')
df2 = pd.read_csv('E1_P2.csv', sep=';')
# Combina los DataFrames en uno solo
df_combined = pd.concat([df1, df2], ignore_index=True)

# Convertir la columna de fecha a tipo datetime en el formato adecuado
df_combined['fecha'] = pd.to_datetime(df_combined['fecha'], format='%d/%m/%Y')

# Convertir la columna de hora a tipo datetime en el formato adecuado
df_combined['hora'] = pd.to_datetime(df_combined['hora'], format='%H:%M:%S', errors='coerce').dt.time

# Eliminar filas con valores NaT en las columnas de fecha y hora
df_combined = df_combined.dropna(subset=['fecha', 'hora'])

# Crear una nueva columna que combine fecha y hora
df_combined['fecha_hora'] = pd.to_datetime(df_combined['fecha'].astype(str) + ' ' + df_combined['hora'].astype(str))

# Ordenar por la columna de fecha y hora combinada
df_combined_sorted = df_combined.sort_values(by=['fecha_hora'])

# Guarda el DataFrame combinado y ordenado en un nuevo archivo CSV con separador por ;
df_combined_sorted.to_csv('archivo_combinado_P1.csv', sep=';', index=False)

# Imprime el DataFrame combinado y ordenado
print(df_combined_sorted)
