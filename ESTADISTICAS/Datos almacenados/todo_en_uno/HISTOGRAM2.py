import pandas as pd
import matplotlib.pyplot as plt

# Cargar el archivo CSV en un DataFrame
df = pd.read_csv('archivo_combinado_modificado_P1.csv', sep=';')

# # Convertir la columna 3 a tipo datetime para facilitar los cálculos
# df['hora'] = pd.to_datetime(df['hora'], format='%H:%M:%S')
# 
# # Calcular la diferencia entre cada fila y la fila anterior en la columna 3
# df['diferencia'] = df['hora'].diff()
# 
# # Calcular el valor absoluto de la diferencia
# df['diferencia_abs'] = df['diferencia'].abs()
# 
# # Convertir la diferencia de tiempo a segundos
# df['diferencia_segundos'] = df['diferencia_abs'].dt.total_seconds()
# 
# # Filtrar los valores de la columna 'diferencia_segundos' que sean mayores a 100
# valores_suprimidos = df.loc[df['diferencia_segundos'] > 100, 'diferencia_segundos']
# 
# # Filtrar los valores de la columna 'diferencia' que sean mayores a 100 segundos
# puntos_mayor_100 = df.loc[df['diferencia_seg'].dt.total_seconds() > 100, 'hora']
# # Mostrar el listado de puntos en los que la diferencia es mayor a 100 segundos
# print("Listado de puntos con diferencia mayor a 100 segundos:")
# print(puntos_mayor_100)
# # Filtrar los valores de la columna 'diferencia_segundos' que sean menores o iguales a 100
# df = df.loc[df['diferencia_segundos'] <= 100]

# Eliminar la primera fila ya que la diferencia será NaN
df = df.dropna()

# Calcular el conteo de valores y sus porcentajes
conteo_valores = df['diferencia_segundos'].value_counts().sort_index()
porcentajes = conteo_valores / conteo_valores.sum() * 100

# Crear el gráfico de barras
fig, ax1 = plt.subplots()

# Graficar los valores absolutos
color = 'tab:blue'
ax1.set_xlabel('Diferencia de tiempo (segundos)')
ax1.set_ylabel('Conteo de ocurrencias', color=color)
ax1.bar(conteo_valores.index, conteo_valores.values, width=1, color=color, alpha=0.6)
ax1.tick_params(axis='y', labelcolor=color)
ax1.set_xlim(0, 100)  # Establecer límites del eje x hasta 100 segundos

# Crear un segundo eje y para los porcentajes
ax2 = ax1.twinx()
color = 'tab:red'
ax2.set_ylabel('Porcentaje de ocurrencias', color=color)
ax2.bar(conteo_valores.index, porcentajes.values, width=1, color=color, alpha=0.3)
ax2.tick_params(axis='y', labelcolor=color)

# Añadir título
plt.title('Histograma de diferencia de tiempo')

# Mostrar el gráfico
fig.tight_layout()
plt.show()
