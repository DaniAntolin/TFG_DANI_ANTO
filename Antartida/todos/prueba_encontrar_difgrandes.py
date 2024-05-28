import pandas as pd
df = pd.read_csv('archivo_combinado3.csv', sep=';')
df['hora'] = pd.to_datetime(df['hora'], format='%H:%M:%S')
df['diferencia'] = df['hora'].diff()
df['diferencia_abs'] = df['diferencia'].abs()
df['diferencia_segundos'] = df['diferencia_abs'].dt.total_seconds()
valores_suprimidos = df.loc[df['diferencia_segundos'] > 100, 'diferencia_segundos']
indices_mayor_100 = df.index[df['diferencia_segundos'] > 100]
df['num_punto'] = 0
for i, idx in enumerate(indices_mayor_100):
    df.at[idx, 'num_punto'] = i + 1

df.to_csv('archivo_combinado_modificado3.csv', sep=';', index=False)
