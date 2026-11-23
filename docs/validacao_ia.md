# Validação cruzada com IA

O enunciado pede dupla. Como não havia dupla na turma, o professor autorizou fazer a atividade **individualmente**, usando **uma IA para validar** o código.

- **IA de validação:** ChatGPT (OpenAI) – revisão independente.

## Como a validação foi feita

1. Enviei ao ChatGPT o enunciado da atividade, todos os arquivos de `src/` e o `testes/PLANO_DE_TESTES.md`.
2. Pedi (prompt usado):
   > Você é revisor de um trabalho de sistemas embarcados (Zephyr, FRDM-KL25Z). Confira se o código atende a cada requisito do enunciado; procure condições de corrida, deadlocks, uso errado de mutex/semáforo e falhas de segurança (pedestre verde com carro não vermelho). Avalie também se o plano de testes cobre os requisitos. Liste os problemas por gravidade, citando arquivo e linha.
3. Cada apontamento foi analisado e classificado como procedente ou não, com a justificativa e a ação tomada.

## Apontamentos

| # | Gravidade (segundo o revisor) | Apontamento da IA revisora | Procede? | Análise e o que foi feito |
|---|---|---|---|---|
| 1 | Alta, se houver falha de GPIO | `hw.c`: os retornos de `gpio_pin_configure()` (em `hw_init()`) e das escritas em `led_set()` são ignorados; o sistema poderia seguir o ciclo com uma saída não configurada. | **Parcialmente** | Procede para a inicialização: `hw_init()` agora confere o retorno de cada `gpio_pin_configure()` e, se algum falhar, imprime erro e retorna `-EIO`; o `main` então não dá a partida e nenhuma thread inicia o ciclo (estado seguro, tudo apagado). Não procede para cada escrita em `led_set()`: após `device_is_ready()` e a configuração bem-sucedida, `gpio_pin_set_raw()` na KL25Z apenas escreve no registrador da porta e não tem caminho de falha; checar cada escrita aumentaria o código sem ganho real. Gravidade "alta" considerada exagerada. |
| 2 | Média – concorrência | Variáveis compartilhadas (`modo`, `pedido_noturno`, `carro_verde_aceso`, `pronto`) usam `volatile`, que não substitui mutex ou operações atômicas; sugere `atomic_t`. | **Não** | Cada uma dessas variáveis é um indicador simples (bool/enum) **escrito por uma única thread** e apenas lido pelas outras. No Cortex-M0+ uma leitura/escrita alinhada de 32 bits é indivisível, então não há valor "pela metade". O `t0_noturno` (64 bits) é escrito antes de um `k_sem_give()` e lido depois do `k_sem_take()` correspondente, que garante a ordem. As decisões de segurança (quem acende quando) são feitas pelos semáforos e mutex, não por esses indicadores. Trocar por `atomic_t` não causaria problema, mas não corrige nenhum defeito; mantido como está. |
| 3 | Média – corrida no limite do verde | `entradas.c` / `carro.c`: se o tempo de 3 s do verde acabar e o botão for apertado antes de `carro_verde_aceso = false`, o log registra "TRAVESSIA SOLICITADA", mas o pedido é descartado no próximo `k_sem_reset()`. | **Sim, sem impacto de segurança** | A análise está correta: existe uma janela de poucos microssegundos em que isso pode acontecer. O efeito é só no registro: nesse caso o verde já está terminando pelo tempo normal, e o amarelo, o vermelho e a travessia do pedestre acontecem logo em seguida do mesmo jeito. Nenhum verde conflitante é liberado (o próprio revisor confirma). Decisão: não alterar o código; comportamento documentado aqui. |
| 4 | Média – teste com prazo incorreto | Plano de testes, TS-N5: a volta completa ao ciclo diurno pode levar ~3 s, não 2 s (a chave é consultada a cada período de 2 s, e depois há espera até o fim do período e a confirmação do pedestre vermelho). | **Sim** | Procede. O evento `MODO DIA` ocorre em até ~2 s após desligar a chave, mas o primeiro carro verde só vem ~1 s depois, quando o pedestre já está vermelho fixo. O resultado esperado do TS-N5 foi corrigido para: "`EVT MODO DIA` em até ~2 s; carro verde em até ~3 s, sempre depois do pedestre vermelho". |
| 5 | Conformidade | `config.h` / `hw.c`: o amarelo usa os mesmos canais vermelho e verde do LED RGB; há três threads para três estados luminosos, mas não três LEDs físicos independentes. | **Sim (documentação)** | Procede como registro de conformidade. É uma adaptação para usar o LED RGB da placa (a mesma do código de exemplo da disciplina): o amarelo é obtido com vermelho + verde. Continua havendo uma thread por estado luminoso e o mutex garante que só um fica ativo por vez. A adaptação foi documentada no `README.md`, com a indicação de como usar três LEDs externos, se necessário (trocar os pinos em `config.h`/`hw.c`). |

## Conclusão

A revisão independente **não encontrou nenhuma falha de segurança**: em nenhum dos apontamentos o pedestre fica verde sem o carro estar vermelho, e não foram apontados deadlocks nem uso incorreto de mutex/semáforos.

Dos cinco apontamentos:

- **1 gerou mudança no código:** checagem dos retornos de `gpio_pin_configure()` em `hw_init()`; em caso de falha, o sistema não inicia o ciclo.
- **2 geraram mudança na documentação:** prazo do TS-N5 corrigido no plano de testes e adaptação do LED RGB registrada no README.
- **2 foram analisados e mantidos como estão, com justificativa:** o uso de `volatile` para indicadores simples (não é defeito nesta arquitetura) e a janela no fim do verde (afeta só o registro do log, sem impacto de segurança).

A validação cruzada foi útil principalmente para melhorar a robustez da inicialização e a precisão do plano de testes. As alterações propostas foram feitas de forma pontual, em vez de reescrever o código, para não introduzir mudanças grandes no código já organizado por fases.
