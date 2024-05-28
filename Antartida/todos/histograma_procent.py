import pandas as pd
import matplotlib.pyplot as plt
df = pd.read_csv('archivo_combinado_modificadoDTtot.csv', sep=';')
df = df.dropna()
conteo_valores = df['diferencia_segundos'].value_counts().sort_index()
porcentajes = conteo_valores / conteo_valores.sum() * 100
fig, ax1 = plt.subplots()
color = 'tab:blue'
ax1.set_xlabel('Diferencia de tiempo (segundos)')
ax1.set_ylabel('Conteo de ocurrencias', color=color)
ax1.bar(conteo_valores.index, conteo_valores.values, width=1, color=color, alpha=0.6)
ax1.tick_params(axis='y', labelcolor=color)
ax1.set_xlim(0, 100)  
ax2 = ax1.twinx()
color = 'tab:red'
ax2.set_ylabel('Porcentaje de ocurrencias', color=color)
ax2.bar(conteo_valores.index, porcentajes.values, width=1, color=color, alpha=0.3)
ax2.tick_params(axis='y', labelcolor=color)
plt.title('Histograma de diferencia de tiempo')
fig.tight_layout()
plt.show()
