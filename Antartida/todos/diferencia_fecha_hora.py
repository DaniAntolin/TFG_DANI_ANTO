import pandas as pd
df = pd.read_csv('archivo_combinado_DT2.csv', sep=';')
df['fecha_hora'] = pd.to_datetime(df['fecha_hora'], format='%Y-%m-%d %H:%M:%S')
df['diferencia'] = df['fecha_hora'].diff()
df['diferencia_abs'] = df['diferencia'].abs()
df['diferencia_segundos'] = df['diferencia_abs'].dt.total_seconds()
valores_suprimidos = df.loc[df['diferencia_segundos'] > 100, 'diferencia_segundos']
indices_mayor_100 = df.index[df['diferencia_segundos'] > 100]
df['num_punto'] = 0
for i, idx in enumerate(indices_mayor_100):
    df.at[idx, 'num_punto'] = i + 1
df.to_csv('archivo_combinado_modificado_DT2.csv', sep=';', index=False)
