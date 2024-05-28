import pandas as pd
import matplotlib.pyplot as plt
# Cargar el archivo CSV en un DataFrame
df = pd.read_csv('archivo_combinado.csv', sep=';')

# Convertir la columna 3 a tipo datetime para facilitar los cálculos
df['hora'] = pd.to_datetime(df['hora'], format='%H:%M:%S')

# Calcular la diferencia entre cada fila y la fila anterior en la columna 3
df['diferencia'] = df['hora'].diff()

# Calcular el valor absoluto de la diferencia
df['diferencia_abs'] = df['diferencia'].abs()

# Convertir la diferencia de tiempo a segundos
df['diferencia_segundos'] = df['diferencia_abs'].dt.total_seconds()

# # Filtrar los valores de la columna 'diferencia_segundos' que sean mayores a 600
# valores_suprimidos = df.loc[df['diferencia_segundos'] > 100, 'diferencia_segundos']
# # Calcular el número de supresiones
# numero_supresiones = len(valores_suprimidos)
# # Calcular la suma de los valores suprimidos
# suma_suprimidos = valores_suprimidos.sum()
# # Imprimir la lista de valores suprimidos
# print("Lista de valores suprimidos:")
# print(valores_suprimidos.tolist())
# # Filtrar los valores de la columna 'diferencia_segundos' que sean menores o iguales a 600
# df = df.loc[df['diferencia_segundos'] <= 100]
# # Imprimir el DataFrame con los valores filtrados
# print("Número de supresiones:", numero_supresiones)
# print("Suma de los valores suprimidos:", suma_suprimidos)

# Imprimir el DataFrame con la nueva columna de diferencia
print(df)

# Eliminar la primera fila ya que la diferencia será NaN
df = df.dropna()

# Guarda el DataFrame combinado y ordenado en un nuevo archivo TXT con separador por ;
#df.to_csv('prueba.csv', sep=';', index=False)

# Obtener los valores únicos de la columna 1 y asignar colores
color_mapping = {'Lower-DT1': 'yellow', 'Top-DT1': 'blue', 'Middle-DT1': 'red'}
# Crear una figura y un eje
plt.figure(figsize=(10, 6))
# Iterar sobre los valores únicos de la columna 1
for label, color in color_mapping.items():
    # Filtrar el DataFrame por la etiqueta específica
    label_df = df[df['Unnamed: 0'] == label]
    # Graficar los puntos correspondientes a esa etiqueta con el color correspondiente
    plt.plot(label_df.index, label_df['diferencia_segundos'], color=color, label=label, marker='o')

# Establecer etiquetas y título
plt.xlabel('Índice de fila')
plt.ylabel('Diferencia de tiempo (segundos)')
plt.title('Diferencia entre cada fila y la fila anterior')
# Unir los puntos
plt.plot(df.index, df['diferencia_segundos'], color='gray', linestyle='-', linewidth=0.5)
# Mostrar la leyenda
plt.legend()
# Mostrar el gráfico
plt.show()

# Contar la cantidad de veces que ocurren diferentes valores de diferencia de tiempo
conteo_valores = df['diferencia_segundos'].value_counts()

# Convertir el resultado en una lista de tuplas (valor, frecuencia)
lista_valores_frecuencia = conteo_valores.reset_index().values.tolist()
# Ordenar la lista de tuplas por el valor de los segundos
lista_valores_frecuencia.sort(key=lambda x: x[0])
# Mostrar la lista de valores y su frecuencia
print("Lista de valores y su frecuencia:")
for valor, frecuencia in lista_valores_frecuencia:
    print(f"Valor: {valor}, Frecuencia: {frecuencia}")
    
# Graficar el histograma de los conteos
plt.bar(conteo_valores.index, conteo_valores.values, width=1)
plt.xlabel('Diferencia de tiempo (segundos)')
plt.ylabel('Cantidad de ocurrencias')
plt.title('Histograma de diferencia de tiempo')
plt.xlim(0, 100)  # Establecer límites del eje x hasta 100 segundos
plt.show()