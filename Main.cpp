#include <opencv2/opencv.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <fstream>


using namespace cv;
using namespace std;
class GameObject {
public:
    Mat image;
    Point position;

    GameObject(const Mat& image, const Point& position)
        : image(image), position(position) {
    }

    void draw(Mat& frame) {
        Rect roi(position, image.size());
        Mat obstacleImageRGBA;
        cvtColor(image, obstacleImageRGBA, COLOR_BGR2BGRA);

        for (int y = 0; y < image.rows; y++) {
            for (int x = 0; x < image.cols; x++) {
                Vec4b pixel = obstacleImageRGBA.at<Vec4b>(y, x);
                if (pixel[3] > 0) { // Verificar se o pixel não é transparente

                    Vec3b& framePixel = frame.at<Vec3b>(position.y + y, position.x + x);
                    framePixel = Vec3b(pixel[0], pixel[1], pixel[2]);
                }
            }
        }
    }
};
//salvar pontuação em arquivo
void salvarPontuacao(int score) {
    std::ofstream arquivo("score.txt");
    if (arquivo.is_open()) {
        arquivo << score;
        arquivo.close();
        std::cout << "Pontuação salva com sucesso!" << std::endl;
    } else {
        std::cout << "Erro ao abrir o arquivo de pontuação." << std::endl;
    }
}

//void gameOverScene() {}
int main() {
    // Carregar o haarcascade para detecção de rosto
    int maxTimeInterval = 5000; // tempo em milisegundos 
    CascadeClassifier faceCascade;
    if (!faceCascade.load("haarcascade_frontalface_default.xml")) {
        cout << "Não foi possível carregar o arquivo XML do classificador de cascata." << std::endl;
        return -1;
    }

    // Carregar a imagem do peixe e verificar se foi carregada corretamente
    Mat fishImage = imread("fish.png", IMREAD_UNCHANGED);
    if (fishImage.empty()) {
        cout << "Não foi possível carregar a imagem do peixe." << std::endl;
        return -1;
    }

    // Carregar a imagem do lixo e verificar se foi carregada corretamente
    Mat obstacleImage = imread("trash.png", IMREAD_UNCHANGED);
    if (obstacleImage.empty()) {
        cout << "Não foi possível carregar a imagem do lixo." << std::endl;
        return -1;
    }

    // Carregar as imagens dos obstáculos adicionais
    Mat sharkImage = imread("shark.png", IMREAD_UNCHANGED);
    if (sharkImage.empty()) {
        cout << "Não foi possível carregar a imagem do tubarão." << std::endl;
        return -1;
    }
    Mat pinkJellyImage = imread("pinkjelly.png", IMREAD_UNCHANGED);
    if (pinkJellyImage.empty()) {
        cout << "Não foi possível carregar a imagem da água-viva rosa." << std::endl;
        return -1;
    }
    Mat blueJellyImage = imread("bluejelly.png", IMREAD_UNCHANGED);
    if (blueJellyImage.empty()) {
        cout << "Não foi possível carregar a imagem da água-viva azul." << std::endl;
        return -1;
    }
    Mat PointImage = imread("point.png", IMREAD_UNCHANGED);
    if (PointImage.empty()) {
        cout << "Não foi possível carregar a imagem do ponto." << std::endl;
        return -1;
    }
    // Iniciar a captura de vídeo da webcam
    VideoCapture capture(0);
    if (!capture.isOpened()) {
        cout << "Não foi possível iniciar a captura de vídeo." << std::endl;
        return -1;
    }
    
    // Configurar o tamanho da janela do jogo
    int windowWidth = capture.get(CAP_PROP_FRAME_WIDTH);
    int windowHeight = capture.get(CAP_PROP_FRAME_HEIGHT);

    // Criar uma janela para exibir o jogo
    namedWindow("FaceFish", WINDOW_NORMAL);
    resizeWindow("FaceFish", windowWidth, windowHeight);

    // Configurar os obstáculos
    vector<GameObject> obstacles;
    int obstacleSpacing = 400;  // Espaçamento entre os obstáculos
    int obstacleSpeed = 5;      // Velocidade dos obstáculos

    // Configurar o objeto ponto
    GameObject point(PointImage, Point(windowWidth, rand() % (windowHeight - PointImage.rows)));

    
    int score = 0;// Variável para controlar a contagem de pontos
    int highScore = 0;  // Variável para armazenar a pontuação mais alta

    // Loop principal do jogo
    while (true) {
        // Capturar o próximo frame da webcam
        Mat frame;
        capture >> frame;

        // Verificar se o frame está vazio (fim da captura)
        if (frame.empty())
            break;
        // Espelhar o frame horizontalmente
        flip(frame, frame, 1);


        // Converter o frame para escala de cinza
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // Detectar rostos no frame usando o haarcascade
        vector<Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.3, 5);
        
       // Desenhar os rostos detectados no frame e centralizar o peixe em cada rosto
        for (const auto& face : faces) {// Calcular a posição do peixe centralizado no rosto
            int fishX = face.x + (face.width - fishImage.cols) / 2;
            int fishY = face.y + (face.height - fishImage.rows) / 2;
            Point fishPosition(fishX, fishY);

            // Criar o objeto do peixe e desenhá-lo no frame
            GameObject fish(fishImage, fishPosition);
            fish.draw(frame);
            
            // Calcular a bounding box do peixe
            Rect fishBoundingBox(fishX, fishY, fishImage.cols, fishImage.rows);

            // Verificar colisão entre o peixe e os obstáculos
            for (auto& obstacle : obstacles) {// Calcular as bounding boxes dos obstáculos
                Rect obstacleBoundingBox(obstacle.position.x, obstacle.position.y, obstacle.image.cols, obstacle.image.rows);

                // Verificar se ocorre colisão entre as caixas dos obstaculos
                if (fishBoundingBox.x < obstacleBoundingBox.x + obstacleBoundingBox.width &&
                    fishBoundingBox.x + fishBoundingBox.width > obstacleBoundingBox.x &&
                    fishBoundingBox.y < obstacleBoundingBox.y + obstacleBoundingBox.height &&
                    fishBoundingBox.y + fishBoundingBox.height > obstacleBoundingBox.y) {
                    //cout<<"VOCE MORREU"<<endl;
                    //cout<<"Sua pontuação foi: "<<score<<endl;
                    salvarPontuacao(highScore);
                }//Remover pontos obstáculos colididos
                obstacles.erase(remove_if(obstacles.begin(), obstacles.end(),
                [fishX, fishY, fishImage](const GameObject& obstacle) {
                    Rect obstacleBoundingBox(obstacle.position.x, obstacle.position.y, obstacle.image.cols, obstacle.image.rows);
                    Rect fishBoundingBox(fishX, fishY, fishImage.cols, fishImage.rows);
                    return fishBoundingBox.x < obstacleBoundingBox.x + obstacleBoundingBox.width &&
                        fishBoundingBox.x + fishBoundingBox.width > obstacleBoundingBox.x &&
                        fishBoundingBox.y < obstacleBoundingBox.y + obstacleBoundingBox.height &&
                        fishBoundingBox.y + fishBoundingBox.height > obstacleBoundingBox.y;
                }),
                obstacles.end());
            }// Verificar colisão entre o peixe e o ponto
            Rect pointBoundingBox(point.position.x, point.position.y, point.image.cols, point.image.rows);
            if (fishBoundingBox.x < pointBoundingBox.x + pointBoundingBox.width &&
                fishBoundingBox.x + fishBoundingBox.width > pointBoundingBox.x &&
                fishBoundingBox.y < pointBoundingBox.y + pointBoundingBox.height &&
                fishBoundingBox.y + fishBoundingBox.height > pointBoundingBox.y) {
                cout << "Colisão com ponto!" << endl;
                score++;
                point.position = Point(windowWidth, rand() % (windowHeight - PointImage.rows));
            }// Remover pontos colididos
            if (fishBoundingBox.x < pointBoundingBox.x + pointBoundingBox.width &&
                fishBoundingBox.x + fishBoundingBox.width > pointBoundingBox.x &&
                fishBoundingBox.y < pointBoundingBox.y + pointBoundingBox.height &&
                fishBoundingBox.y + fishBoundingBox.height > pointBoundingBox.y) {
                obstacles.erase(remove_if(obstacles.begin(), obstacles.end(),
                    [pointBoundingBox](const GameObject& obstacle) {
                        Rect obstacleBoundingBox(obstacle.position.x, obstacle.position.y, obstacle.image.cols, obstacle.image.rows);
                        return pointBoundingBox.x < obstacleBoundingBox.x + obstacleBoundingBox.width &&
                            pointBoundingBox.x + pointBoundingBox.width > obstacleBoundingBox.x &&
                            pointBoundingBox.y < obstacleBoundingBox.y + obstacleBoundingBox.height &&
                            pointBoundingBox.y + pointBoundingBox.height > obstacleBoundingBox.y;
                    }),
                    obstacles.end());
            }
        }// Verificar se a pontuação atual é maior que a pontuação mais alta
        if (score > highScore) {
        highScore = score;
        salvarPontuacao(highScore);
        }
        
        // Desenhar os obstáculos no frame
        for (auto& obstacle : obstacles) {
            obstacle.draw(frame);
            obstacle.position.x -= obstacleSpeed;
        }// Desenhar o ponto no frame
        point.draw(frame);
        point.position.x -= obstacleSpeed;

        // Adicionar novos obstáculos
        if (obstacles.empty() || obstacles.back().position.x <= windowWidth - obstacleSpacing) {
            Point obstaclePosition(windowWidth, rand() % (windowHeight - obstacleImage.rows));
            obstacles.emplace_back(sharkImage, obstaclePosition);
            if (score >= 10) {
            Point obstaclePosition1(windowWidth, rand() % (windowHeight - obstacleImage.rows));
            obstacles.emplace_back(obstacleImage, obstaclePosition1);
            }   
            if (score >= 15) {
            Point obstaclePosition2(windowWidth, rand() % (windowHeight - obstacleImage.rows));
            obstacles.emplace_back(pinkJellyImage, obstaclePosition2);
            }   
            if (score >= 20) {
            Point obstaclePosition3(windowWidth, rand() % (windowHeight - obstacleImage.rows));
            obstacles.emplace_back(blueJellyImage, obstaclePosition3);
            }   
            if (score % 2 == 0) {
            // Adicione um novo ponto
            Point pointPosition(windowWidth, rand() % (windowHeight - PointImage.rows));
            obstacles.emplace_back(PointImage, pointPosition);
        }
        }// Exibir o frame resultante
        imshow("FaceFish", frame);
        
        // Verificar se a tecla 'Esc' foi pressionada para sair do jogo
        if (waitKey(1) == 27)
            break;
        
    }// Encerrar a captura de vídeo e fechar a janela do jogo
    capture.release();
    destroyAllWindows();

    return 0;
}
