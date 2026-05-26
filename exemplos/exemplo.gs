Start {
    Level ataque = 10;
    T4 chance = 50;
    T5 dano = 90;
    Meta ativo = 1;
    Lore nome = "texto";
    Assinatura fixo = 1;
    Anunciar(nome);
    Coletar(ataque);
    50/50 (ataque >= 10 E ativo == 1) {
        Drop ataque;
    } Garantido {
        Farmar(i = 0; i < 10; i = i + 1) {
            Combo(i < 5 OU ataque != 0) {
                Seletor(i) {
                    Quebra_Fraqueza;
                }
            }
        }
    }
}
j@
1a
2.a3
5555555555555555
minha_variavel_para_testar_um_nome_muito_longo = 1
texto_ruim = "hello world
char_ruim = 'a
@
/* comentario sem fechamento