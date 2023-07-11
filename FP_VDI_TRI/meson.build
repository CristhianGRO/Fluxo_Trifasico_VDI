project('FP_VDI_TRI', 'c')

SourceFiles = [
    'src/main.c',
    'src/aemt.c',
    'src/funcoesAuxiliares.c',
    'src/funcoesChaveamento.c',
    'src/funcoesHeuristica.c',
    'src/funcoesInicializacao.c',
    'src/funcoesLeituraDados.c',
    'src/funcoesProblema.c',
    'src/funcoesRNP.c',
    'src/funcoesSetor.c',
]

executable(
    'FP_VDI_TRI',
    SourceFiles,
    link_args: ['-lm', '-pthread']
)
