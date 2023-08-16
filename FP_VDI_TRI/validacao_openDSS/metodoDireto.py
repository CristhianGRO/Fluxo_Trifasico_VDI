#Algoritmo para simulação de redes trifasicas via OpenDSS pelo método direto
#Autor: Cristhian Gabriel da Rosa de Oliveira
#Data: 09/08/2023
#=================================================================

#Movendo a execucao para a pasta raiz do algoritmo
from pathlib import Path
RAIZ = Path(__file__).parent.resolve().absolute()

#Importando as bibliotecas
import py_dss_interface

#Criando o objeto OpenDSS
dss = py_dss_interface.DSSDLL()

dss.text("Clear")
dss.text("Set DefaultBaseFrequency = 60")

# Atencao!!! Preencha as informacoes abaixo antes de rodar o algoritmo
modelo = "IEEE_4BUS"
baseTensao_kV = 13.8


#=================================================================
#Inicio do algoritmo OpenDSS com os dados definidos

#1) Definindo o circuito
dss.text("New circuit.simulacao bus1=barra1.1.2.3 basekv={} pu=1.0 angle=0 phases=3 mvasc1= 99999999 mvasc3= 99999999".format(baseTensao_kV))

#2) Definindo a base de tensao
dss.text("Set voltagebases=[ {} ]".format(baseTensao_kV))
dss.text("calcvoltagebases")

#3) Lendo e armazenando os dados de entrada
dadosBarra = []
dadosTrecho = []
nBarras = 0
nTrecho = 0

DIVISOR_POTENCIA = 1000
#Dados de Barra:
def leituraBarras():
    dataFile = open(RAIZ/f"{modelo}/data_bus.csv","r")
    linhas = dataFile.readlines()
    j=0
    for linha in linhas:
        if j != 0:
            id=int(linha.split(",",2)[0])
            fases=int(linha.split(",",2)[1])
            dadosBarra.append([])
            dadosBarra[j-1].append(id)
            dadosBarra[j-1].append(fases)
            Pa=float(linha.split(",",5)[2])/DIVISOR_POTENCIA
            Qa=float(linha.split(",",5)[3])/DIVISOR_POTENCIA
            dadosBarra[j-1].append(Pa)
            dadosBarra[j-1].append(Qa)
            Pb=float(linha.split(",",5)[4])/DIVISOR_POTENCIA
            Qb=float(linha.split(",",8)[5])/DIVISOR_POTENCIA
            dadosBarra[j-1].append(Pb)
            dadosBarra[j-1].append(Qb)
            Pc=float(linha.split(",",8)[6])/DIVISOR_POTENCIA
            Qc=float(linha.split(",",8)[7])/DIVISOR_POTENCIA
            dadosBarra[j-1].append(Pc)
            dadosBarra[j-1].append(Qc)
        else:
            nBarras=int(linha.split(",",2)[0])
        j=j+1
    return nBarras

nBarras    = leituraBarras()

DIVISOR_IMPEDANCIA = 1
#Dados de Linha:
def leituraLinhas():
    dataFile = open(RAIZ/f"{modelo}/data_branch.csv","r")
    linhas = dataFile.readlines()
    j=0
    for linha in linhas:
        if j != 0:
            id=int(linha.split(",",2)[0])
            barraDe=int(linha.split(",",3)[1])
            barraPara=int(linha.split(",",3)[2])
            ampacidade=float(linha.split(",",5)[3])
            dadosTrecho.append([])
            dadosTrecho[j-1].append(id)
            dadosTrecho[j-1].append(barraDe)
            dadosTrecho[j-1].append(barraPara)
            dadosTrecho[j-1].append(ampacidade)

            raa=float(linha.split(",",18)[4])/DIVISOR_IMPEDANCIA
            xaa=float(linha.split(",",18)[5])/DIVISOR_IMPEDANCIA
            dadosTrecho[j-1].append(raa)
            dadosTrecho[j-1].append(xaa)
            rab=float(linha.split(",",18)[6])/DIVISOR_IMPEDANCIA
            xab=float(linha.split(",",18)[7])/DIVISOR_IMPEDANCIA
            dadosTrecho[j-1].append(rab)
            dadosTrecho[j-1].append(xab)
            rac=float(linha.split(",",18)[8])/DIVISOR_IMPEDANCIA
            xac=float(linha.split(",",18)[9])/DIVISOR_IMPEDANCIA
            dadosTrecho[j-1].append(rac)
            dadosTrecho[j-1].append(xac)
            rbb=float(linha.split(",",18)[10])/DIVISOR_IMPEDANCIA
            xbb=float(linha.split(",",18)[11])/DIVISOR_IMPEDANCIA
            dadosTrecho[j-1].append(rbb)
            dadosTrecho[j-1].append(xbb)
            rbc=float(linha.split(",",18)[12])/DIVISOR_IMPEDANCIA
            xbc=float(linha.split(",",18)[13])/DIVISOR_IMPEDANCIA
            dadosTrecho[j-1].append(rbc)
            dadosTrecho[j-1].append(xbc)
            rcc=float(linha.split(",",18)[14])/DIVISOR_IMPEDANCIA
            xcc=float(linha.split(",",18)[15])/DIVISOR_IMPEDANCIA
            dadosTrecho[j-1].append(rcc)
            dadosTrecho[j-1].append(xcc)
        else:
            nTrecho=int(linha.split(",",2)[0])
        j=j+1
    return nTrecho

nTrecho    = leituraLinhas()

#4) Iniciando os dados de linha
#Insere os elementos na ordem da submatriz inferior da matriz Zabc, com espaçamento das linhas por "|"
#Rmatrix = "11 | 21 22 | 31 32 33"
#Xmatriz = "11 | 21 22 | 31 32 33"

#Para fins de generalidade, considera-se cada linha como um único linecode

for i in range(nTrecho):
    dss.text("New linecode.L{} basefreq=60 units=km".format(i+1))
    dss.text('~ rmatrix="{} | {} {} | {} {} {}" xmatrix="{} | {} {} | {} {} {}"'.format(dadosTrecho[i][4],dadosTrecho[i][6],dadosTrecho[i][10],dadosTrecho[i][8],dadosTrecho[i][12],dadosTrecho[i][14],dadosTrecho[i][5],dadosTrecho[i][7],dadosTrecho[i][11],dadosTrecho[i][9],dadosTrecho[i][13],dadosTrecho[i][15]))
    #print(f"Linha {i+1}, com rmatrix = {dadosTrecho[i][4]} {dadosTrecho[i][6]} {dadosTrecho[i][10]} {dadosTrecho[i][8]} {dadosTrecho[i][12]} {dadosTrecho[i][14]} e xmatrix = {dadosTrecho[i][5]} {dadosTrecho[i][7]} {dadosTrecho[i][11]} {dadosTrecho[i][9]} {dadosTrecho[i][13]} {dadosTrecho[i][15]} criada com sucesso")

#5) Descrevendo as linhas do Modelo
for i in range(nTrecho):
    dss.text("New line.Linha_{} Bus1=barra{}.1.2.3 Bus2=barra{}.1.2.3 length=1 units=km linecode=L{}".format(i+1,dadosTrecho[i][1],dadosTrecho[i][2],i+1))
    #print(f"linha {i+1}, Barra de = {dadosTrecho[i][1]} Barra Para = {dadosTrecho[i][2]} criada com sucesso")

#6) Descrevendo as cargas do Modelo
#Model = 1: modelo de potencia constante
import numpy as np
for i in range(nBarras):
    dss.text("New Load.{}a Phases = 1 kV= {}  Bus1=barra{}.1 Model=1  kW= {} kvar= {}".format(i+1,baseTensao_kV/np.sqrt(3), dadosBarra[i][0],dadosBarra[i][2],dadosBarra[i][3]))
    dss.text("New Load.{}b Phases = 1 kV= {}  Bus1=barra{}.2 Model=1  kW= {} kvar= {}".format(i+1,baseTensao_kV/np.sqrt(3), dadosBarra[i][0],dadosBarra[i][4],dadosBarra[i][5]))
    dss.text("New Load.{}c Phases = 1 kV= {}  Bus1=barra{}.3 Model=1  kW= {} kvar= {}".format(i+1,baseTensao_kV/np.sqrt(3), dadosBarra[i][0],dadosBarra[i][6],dadosBarra[i][7]))
    
    #print(f"Carga {i+1} criada com sucesso com kW.1 = {dadosBarra[i][2]}, kW.2 = {dadosBarra[i][4]}, kW.3 = {dadosBarra[i][6]} e kvar.1 = {dadosBarra[i][3]}, kvar.2 = {dadosBarra[i][5]}, kvar.3 = {dadosBarra[i][7]}")

#7) Executando o algoritmo

dss.text("calcvoltagebases")
dss.text("Solve")
dss.text("Show Voltages LN Nodes")
dss.text("Show Currents elem")
#dss.text("Show Powers kVA elements")
dss.text("plot profile")
