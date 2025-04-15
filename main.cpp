#include "PIG.h"


struct Vetor{
    double x, y;
};


double distancia_entre_pontos(const Vetor ponto1, const Vetor ponto2) {
    const double distancia = sqrt((ponto1.x-ponto2.x)*(ponto1.x-ponto2.x) + (ponto1.y-ponto2.y)* (ponto1.y-ponto2.y));
    return distancia;
}


void atualizarCamera(const PIGTeclado teclado)
{
    double velocidadeTranslado = 10;
    double velocidadeZoom = 0.01;
    if(teclado[PIG_TECLA_w])     /// Cima
    {
        DeslocaCamera(0, velocidadeTranslado, 0);
    }
    if(teclado[PIG_TECLA_s])     /// Baixo
    {
        DeslocaCamera(0, -velocidadeTranslado, 0);
    }
    if(teclado[PIG_TECLA_a])     /// Esquerda
    {
        DeslocaCamera(-velocidadeTranslado, 0, 0);
    }
    if(teclado[PIG_TECLA_d])     /// Direita
    {
        DeslocaCamera(velocidadeTranslado, 0, 0);
    }
    if(teclado[PIG_TECLA_q])     /// Aumentar zoom
    {
        SetAfastamentoCamera(GetAfastamentoCamera()+velocidadeZoom);
    }
    if(teclado[PIG_TECLA_e])     /// Diminuir zoom
    {
        SetAfastamentoCamera(GetAfastamentoCamera()-velocidadeZoom);
    }
}


class Corpo {
    int sprite_id;
public:
    Vetor aceleracao{0, 0};
    Vetor velocidade{0, 0};
    double raio;
    double massa;
    Vetor posicao;
    Corpo(const double massa, const double raio, const Vetor posicao, const int sprite_id) {
        this->massa = massa;
        this->raio = raio;
        this->posicao = posicao;
        this->sprite_id = sprite_id;
        SetDimensoesSprite(sprite_id, 2*raio, 2*raio);
    }
    void atualizar() {
        velocidade.x += aceleracao.x;
        velocidade.y += aceleracao.y;
        posicao.x += velocidade.x;
        posicao.y += velocidade.y;
    }
    void desenhar() const {
        MoveSprite(sprite_id, posicao.x-raio, posicao.y-raio);
        DesenhaSprite(sprite_id);
    }
};


void aplicarColisao(Corpo* p1, Corpo* p2)
{
    double m1 = p1->massa;
    double m2 = p2->massa;

    double u1x = p1->velocidade.x;
    double u1y = p1->velocidade.y;
    double u2x = p2->velocidade.x;
    double u2y = p2->velocidade.y;

    double x1 = p1->posicao.x;
    double y1 = p1->posicao.y;
    double x2 = p2->posicao.x;
    double y2 = p2->posicao.y;

    double u1 = sqrt(u1x*u1x + u1y*u1y);
    double u2 = sqrt(u2x*u2x + u2y*u2y);

    double a1 = atan2(y2-y1, x2-x1);
    double b1 = atan2(u1y, u1x);
    double c1 = b1-a1;

    double a2 = atan2(y1-y2, x1-x2);
    double b2 = atan2(u2y, u2x);
    double c2 = b2-a2;

    double u12 = u1*cos(c1);
    double u11 = u1*sin(c1);

    double u21 = u2*cos(c2);

    double v12 = (((m1-m2)*u12) - (2*m2*u21))/(m1+m2);

    double v1x = u11*(-sin(a1)) + v12*(cos(a1));
    double v1y = u11*(cos(a1)) + v12*(sin(a1));

    p1->velocidade.x = v1x;
    p1->velocidade.y = v1y;
}


class Simulacao {
    Corpo* planeta;
    int quantidadeProjeteis = 0;
    static constexpr int quantidadeMaximaProjeteis = 255;
    Corpo* projeteis[quantidadeMaximaProjeteis];
    const double CONSTANTE_GRAVITACIONAL = 0.1;
    const double COEFICIENTE_RESTITUICAO = 0.9;
    double velocidadeHorizontalProjetil = 5;
    int sprite_torre;
    int sprite_canhao;
    int sprite_projetil;
    int sprite_planeta;
public:
    Simulacao() {
        char caminho_torre[] = "../imagens/torre.png";
        sprite_torre = CriaSprite(caminho_torre, 0);
        SetDimensoesSprite(sprite_torre, 108, 192);
        char caminho_canhao[] = "../imagens/canhao.png";
        sprite_canhao = CriaSprite(caminho_canhao, 0);
        SetDimensoesSprite(sprite_canhao, 25, 25);
        char caminho_projetil[] = "../imagens/projetil.png";
        sprite_projetil = CriaSprite(caminho_projetil, 0);
        char caminho_planeta[] = "../imagens/terraNoite.png";
        sprite_planeta = CriaSprite(caminho_planeta, 0);

        planeta = new Corpo(1000000, 350,
            {GetLarguraJanela()/2.0,
                GetAlturaJanela()/2.0}, sprite_planeta);
        MoveSprite(sprite_torre, planeta->posicao.x-96,
            planeta->posicao.y + planeta->raio-52);
        MoveSprite(sprite_canhao, planeta->posicao.x-22,
            planeta->posicao.y+planeta->raio+5);
    }
    ~Simulacao() {
        DestroiSprite(sprite_canhao);
        DestroiSprite(sprite_torre);
        DestroiSprite(sprite_projetil);
        DestroiSprite(sprite_planeta);
    }
    void criar_projetil() {
        if (quantidadeProjeteis < quantidadeMaximaProjeteis) {
            const double x = planeta->posicao.x;
            const double y = planeta->posicao.y + planeta->raio + 24;
            projeteis[quantidadeProjeteis] = new Corpo(1, 2, {x, y}, sprite_projetil);
            projeteis[quantidadeProjeteis]->velocidade = {velocidadeHorizontalProjetil, 0};
            quantidadeProjeteis += 1;
        }
    }
    void mudar_velocidade_projetil(const double variacao) {
        velocidadeHorizontalProjetil += variacao;
    }
    void atualizar() {
        planeta->atualizar();
        for (int i = 0; i < quantidadeProjeteis; i++) {
            const double projetil_x = projeteis[i]->posicao.x;
            const double projetil_y = projeteis[i]->posicao.y;
            const double distancia = distancia_entre_pontos(planeta->posicao, projeteis[i]->posicao);
            const double forcaGravitacional = CONSTANTE_GRAVITACIONAL*projeteis[i]->massa*planeta->massa/(distancia*distancia);
            const double cos = (planeta->posicao.x-projetil_x)/distancia;
            const double sen = (planeta->posicao.y-projetil_y)/distancia;
            const double fx = forcaGravitacional * cos;
            const double fy = forcaGravitacional * sen;
            const double ax = fx / projeteis[i]->massa;
            const double ay = fy / projeteis[i]->massa;
            projeteis[i]->aceleracao = {ax, ay};
            projeteis[i]->atualizar();
            if (distancia_entre_pontos(projeteis[i]->posicao, planeta->posicao) < projeteis[i]->raio+planeta->raio) {
                projeteis[i]->posicao = {projetil_x, projetil_y};
                projeteis[i]->velocidade.x *= COEFICIENTE_RESTITUICAO;
                projeteis[i]->velocidade.y *= COEFICIENTE_RESTITUICAO;
                aplicarColisao(projeteis[i], planeta);
            }
        }
    }
    void desenhar() {
        planeta->desenhar();
        DesenhaSprite(sprite_canhao);
        DesenhaSprite(sprite_torre);
        for (int i = 0; i < quantidadeProjeteis; i++) {
            projeteis[i]->desenhar();
        }
    }
};


int WinMain() {
    CriaJogo("Canhão de Newton",0, 1100, 1920);
    const int timer = CriaTimer();
    int rodando = 1;
    auto* simulacao = new Simulacao;
    PreparaCameraMovel();
    while (rodando) {
        if (TempoDecorrido(timer) > 0.016) {
            rodando = JogoRodando();
            GetEvento();
            PIGTeclado teclado = GetTeclado();
            if (teclado[PIG_TECLA_ESC]) {
                rodando = 0;
            } else if (teclado[PIG_TECLA_ENTER]) {
                simulacao->criar_projetil();
            } else if (teclado[PIG_TECLA_CIMA]) {
                simulacao->mudar_velocidade_projetil(0.5);
            } else if (teclado[PIG_TECLA_BAIXO]) {
                simulacao->mudar_velocidade_projetil(-0.5);
            }
            atualizarCamera(teclado);
            simulacao->atualizar();
            IniciaDesenho();
            simulacao->desenhar();
            EncerraDesenho();
            ReiniciaTimer(timer);
        }
    }
    delete simulacao;
    DestroiTimer(timer);
    FinalizaJogo();
    return 0;
}