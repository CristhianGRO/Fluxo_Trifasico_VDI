<div align="center">
    <img src=".\.github\Imagens\linhaTrifasica.png" style="width: 50vw">
    
     Fluxo de Carga Trifásico pelo Método de 
     Varredura Direta Inversa, utilizando a 
     estruturas de dados de relação nó-profundidade.
</div>

<br><br>

# ⛅ Introdução

O presente repositório apresenta um algoritmo de fluxo de carga trifásico pelo método de varreduta direta-inversa, utilizando a estruturas de dados RNP (relação nó-profundidade), tornando-o computacionalmente eficiente. 

O objetivo desse material é possibilitar que outros pesquisadores do tema utilizem o algoritmo para o desenvolvimento de novas pesquisas envolvendo o tema.

<br><br>

# 🗂️ Organização

O algoritmo pode ser encontrado na pasta `FP_VDI_TRI`. Dentro dela, os arquivos estão organizados da seguinte forma:

## Raiz

Os arquivos de extensão `.dad` presentes na raiz da pasta `FP_VDI_TRI` referem-se aos dados de entrada utilizados para a execução do fluxo de potência. Dentre eles, citam-se como essenciais:

- barras.dad
- barrasTrifasicas.dad
- chaves.dad
- chavesCompleto.dad
- dadosTrafo.dad
- dadosTrafoTrifasicos.dad
- dados_alimentadores.dad
- grafosetores.dad
- matrizImpedanciaCorrenteTrifasicos.dad
- rnpsetores.dad

## Pasta: src

Aqui serão encontrados os arquivos `.c` que contém os algoritmos desenvolvidos, além dos arquivos `.h` referentes aos _headers_ desses algoritmos.

O algoritmo pode ser entendido iniciando-se o estudo pelo arquivo `main.c`, e seguindo o fluxo de informação entre as funções.

- Nota 1: o algoritmo foi alterado com base no Reconfigurador Monofásico desenvolvido ao longo do doutorado do prof. Dr. Leandro Tolomeu Marques. As funções adaptadas para que o fluxo possa considerar dados trifásicos estão todas presentes no arquivo `funcoesTrifasicas.c`.

- Nota 2: até o momento, as funções referentes ao reconfigurador não foram atualizadas para dados trifásicos, e portanto não são utilizadas. Muitas estão comentadas para que não acuse erro no compilador. Esteja ciente que deverão ser atualizadas posteriormente, seguindo a mesma filosofia.

## Pasta: Modelos

Dentro dessa pasta armazenam-se os dados dos modelos trifásicos que serão simulados. É importante salientar que essa pasta serve apenas para armazenamento, dos arquivos, devendo esses dados serem copiados para a raiz da pasta `FP_VDI_TRI` antes da simulação. 

Implementações futuras podem receber o nome do modelo que deseja-se simular e buscar, automaticamente, na pasta correspondente, sem a necessidade desse trabalho manual.
## Pasta: validacao_openDSS

Essa pasta possui um algoritmo em python `metodoDireto.py`, o qual possui a finalidade de ler os dados de entrada presentes na pasta com nome de um modelo que deseja-se simular (atualmente possuindo a pasta IEEE_4BUS), cujo nome é inserido no início do algoritmo. 

Esse algoritmo lê os dados de barra (presentes no arquivo `data_bus.csv`) e dados de linha (`data_branch.csv`) e gera o código para simulação em openDSS automaticamente.

Para utilizá-lo é necessário que se tenha instalado o python em sua máquina, juntamente com o pip, além da biblioteca py-dss-interface. Para poder baixar essa biblioteca, abra um terminal e digite o código abaixo:

~~~

pip install py-dss-interface

~~~

A biblioteca será baixada e o código python poderá ser executado normalmente.

<br><br>

# 🖥️ Como simular?

Antes da primeira simulação, instale alguns programas e bibliotecas necessários:
## Windows
- Instale o [MSYS2](https://www.msys2.org/)
- Abra um terminal do MSYS2 e instale dependências:
  - `pacman -Sy base-devel python3 python3-pip meson ninja mingw-w64-x86_64-gcc mingw-w64-x86_64-winpthreads gdb`

  - `pip install poetry`

- Abra o MSYS2, mude o diretório para o MRANPP `cd /c/Users/Usuário/.../MRANPP`
- `poetry install`

Isso é só necessário na primeira simulação. Para simular, abra um terminal na pasta `FP_VDI_TRI` e execute o simples comando:

~~~
run.bat
~~~

O algoritmo irá compilar os arquivos e realizar a execução dos códigos automaticamente.

# 💾 Download
Baixe os dados deste repositório em formato `.zip` [através deste link](https://github.com/CristhianGRO/Fluxo_Trifasico_VDI/archive/refs/heads/main.zip)


<br><br>

# ✉️ Contato

Dúvidas ou sugestões? Entre em contato: 

- **E-mail:** cristhiangro@gmail.com

<br><br>

# 💬 Cite este repositório

```bibtex
@article{FP_VDI_TRI,
    title    = {Fluxo de Carga Trifásico pelo Método de Varredura Direta Inversa, utilizando a estruturas de dados de relação nó-profundidade.},
    year     = {2023},
    url      = {https://github.com/CristhianGRO/Fluxo_Trifasico_VDI},
    author   = {Cristhian Gabriel da Rosa de Oliveira},
    keywords = {Redes de Distribuição; Sistemas Elétricos de Potência; Fluxo de Carga Trifásico.},
}
```
