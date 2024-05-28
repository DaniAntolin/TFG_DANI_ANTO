import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv('archivo_combinado.csv', sep=';')
df['hora'] = pd.to_datetime(df['hora'], format='%H:%M:%S')
df['diferencia'] = df['hora'].diff()
df['diferencia_abs'] = df['diferencia'].abs()
df['diferencia_segundos'] = df['diferencia_abs'].dt.total_seconds()
print(df)
df = df.dropna()
color_mapping = {'Lower-DT1': 'yellow', 'Top-DT1': 'blue', 'Middle-DT1': 'red'}
plt.figure(figsize=(10, 6))
for label, color in color_mapping.items():
    label_df = df[df['Unnamed: 0'] == label]
    plt.plot(label_df.index, label_df['diferencia_segundos'], color=color, label=label, marker='o')
plt.xlabel('Índice de fila')
plt.ylabel('Diferencia de tiempo (segundos)')
plt.title('Diferencia entre cada fila y la fila anterior')
plt.plot(df.index, df['diferencia_segundos'], color='gray', linestyle='-', linewidth=0.5)
plt.legend()
plt.show()
conteo_valores = df['diferencia_segundos'].value_counts()
lista_valores_frecuencia = conteo_valores.reset_index().values.tolist()
lista_valores_frecuencia.sort(key=lambda x: x[0])
print("Lista de valores y su frecuencia:")
for valor, frecuencia in lista_valores_frecuencia:
    print(f"Valor: {valor}, Frecuencia: {frecuencia}")
plt.bar(conteo_valores.index, conteo_valores.values, width=1)
plt.xlabel('Diferencia de tiempo (segundos)')
plt.ylabel('Cantidad de ocurrencias')
plt.title('Histograma de diferencia de tiempo')
plt.xlim(0, 100) 
plt.show()