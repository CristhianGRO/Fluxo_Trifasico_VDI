#Algoritmo para simulação de redes trifasicas via OpenDSS pelo método indireto
#Autor: Cristhian Gabriel da Rosa de Oliveira
#Data: 09/08/2023
#=======================================================================

#Importando as bibliotecas
import py_dss_interface

#Criando o objeto OpenDSS
dss = py_dss_interface.DSSDLL()

#AVISO: Antes de rodar  
dss_file = r"C:\Users\PC-01\OneDrive\Pastas\UFMT\TCC\Rede Tocantins\Modelagem ZABC Rede Primaria\ZABC_Rede_Primaria.dss"

dss.text("compile [{}]".format(dss_file))