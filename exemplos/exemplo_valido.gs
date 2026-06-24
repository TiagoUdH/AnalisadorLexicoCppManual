Start {
    Level pity = 0;
    Level limite = 90;
    Level ganhou_50_50 = 0;

    Farmar(pity = 0; pity < limite; pity = pity + 1) {
        Anunciar("Realizando desejo no banner...");

        50/50 (pity == 89) {
            Anunciar("Brilhou dourado!");
            Drop;
        } Garantido {
            Combo(pity < 5 OU pity != 0) {
                Seletor(pity) {
                    Anunciar("Sistema de pity ativo");
                    Quebra_Fraqueza;
                }
            }
        }
    }

    Anunciar("Total de desejos realizados: ");
    Anunciar(pity);
}
