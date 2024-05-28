import pandas as pd
df1 = pd.read_csv('AWS_9m_1min_2024_03-12a14-soloDT2.csv', sep=';')
df2 = pd.read_csv('AWS_3m_1min_2024_03-12a14-soloDT2.csv', sep=';')
df3 = pd.read_csv('AWS_1m_1min_2024_03-12a14-soloDT2.csv', sep=';')
df_combined = pd.concat([df1, df2, df3], ignore_index=True)
df_combined['fecha'] = pd.to_datetime(df_combined['fecha'], format='%d/%m/%Y')
df_combined['hora'] = pd.to_datetime(df_combined['hora'], format='%H:%M:%S').dt.time
df_combined['fecha_hora'] = pd.to_datetime(df_combined['fecha'].astype(str) + ' ' + df_combined['hora'].astype(str))
df_combined_sorted = df_combined.sort_values(by=['fecha_hora'])
df_combined_sorted.to_csv('archivo_combinado_DT2.csv', sep=';', index=False)
print(df_combined_sorted)
