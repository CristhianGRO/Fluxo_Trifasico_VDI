project('FP_VDI_TRI', 'c')

SourceFiles = [
    'src/main.c',
    'src/funcoesAuxiliares.c',
    'src/funcoesChaveamento.c',
    'src/funcoesRNP.c',
    'src/funcoesInicializacao.c',
    'src/funcoesLeituraDados.c',
    'src/funcoesHeuristica.c',
    'src/funcoesProblema.c',
    'src/funcoesSetor.c',
    'src/funcoesTrifasicas.c',
]

# Desabilitar -Wunused-but-set-variable
add_global_link_arguments(['-Wno-unused-variable'], language: 'c')

# Desabilitar -Wunused-result
add_global_arguments('-Wno-unused-result', language: 'c')

# Desabilitar -Wunused-but-not-set-variable
add_global_arguments('-Wno-unused-but-set-variable', language: 'c')

# Desabilitar -Wstringop-overflow=
add_global_arguments('-Wno-stringop-overflow', language: 'c')

# Desabilitar -Wmaybe-unitialized warning
add_global_arguments('-Wno-maybe-uninitialized', language: 'c')

# Desabilitar -Wswitch
add_global_arguments('-Wno-switch', language: 'c')

# Desabilitar -Wdangling-else
add_global_arguments('-Wno-dangling-else', language: 'c')

# Desabilitar -Wunused-variable
add_global_arguments('-Wno-unused-variable', language: 'c')

executable(
    'FP_VDI_TRI',
    SourceFiles,
    link_args: ['-lm']
)
