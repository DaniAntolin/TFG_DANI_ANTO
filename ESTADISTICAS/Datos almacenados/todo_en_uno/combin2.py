import pandas as pd

df1 = pd.read_csv('E1_P1.csv', sep=';')
df2 = pd.read_csv('E1_P2.csv', sep=';')
df_combined = pd.concat([df1, df2], ignore_index=True)
df_combined['fecha'] = pd.to_datetime(df_combined['fecha'], format='%d/%m/%Y')
df_combined['hora'] = pd.to_datetime(df_combined['hora'], format='%H:%M:%S', errors='coerce').dt.time
df_combined = df_combined.dropna(subset=['fecha', 'hora'])
df_combined['fecha_hora'] = pd.to_datetime(df_combined['fecha'].astype(str) + ' ' + df_combined['hora'].astype(str))
df_combined_sorted = df_combined.sort_values(by=['fecha_hora']
df_combined_sorted.to_csv('archivo_combinado_P1.csv', sep=';', index=False)
print(df_combined_sorted)
