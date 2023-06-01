#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(){
    // Carregar o Haar Cascade para detecção de rosto
    CascadeClassifier faceCascade;
    faceCascade.load("haarcascade_frontalface_default.xml");

    // Carregar a imagem do peixe
    Mat fishImage = imread("fish.png", IMREAD_UNCHANGED);

    // Inicializar a captura de vídeo da webcam
    VideoCapture capture(0);
    if (!capture.isOpened()){
        cout << "Erro ao abrir a webcam" << std::endl;
        return 1;
    }

    namedWindow("Jogo", WINDOW_NORMAL);
    resizeWindow("Jogo", 800, 600);  // Definir o tamanho desejado para a janela


    while (true){
        Mat frame;
        capture >> frame;

        // Criar um quadrado transparente azulado que cobre a tela inteira
        Mat overlay = frame.clone();
        rectangle(overlay, Point(0, 0), Point(frame.cols, frame.rows), Scalar(250, 241, 122, 128), -1);
        addWeighted(overlay, 0.5, frame, 0.5, 0, frame);

        // Converter a imagem para escala de cinza
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // Detectar os rostos na imagem
        vector<Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces);

        // Desenhar um retângulo ao redor de cada rosto detectado e colocar a imagem do peixe centralizada no rosto
        for (const Rect& faceRect : faces) {
            // Calcular as coordenadas para posicionar o peixe centralizado no rosto
            int fishX = faceRect.x + (faceRect.width - fishImage.cols) / 2;
            int fishY = faceRect.y + (faceRect.height - fishImage.rows) / 2;

            // Verificar se o peixe cabe dentro do retângulo do rosto
            if (fishX >= 0 && fishY >= 0 && fishX + fishImage.cols <= frame.cols && fishY + fishImage.rows <= frame.rows) {
                // Copiar a imagem do peixe para a região do rosto na imagem original
                Mat roi = frame(Rect(fishX, fishY, fishImage.cols, fishImage.rows));
                Mat fishImageRGBA;
                cvtColor(fishImage, fishImageRGBA, COLOR_BGR2BGRA);

                for (int y = 0; y < fishImage.rows; y++) {
                    for (int x = 0; x < fishImage.cols; x++) {
                        Vec4b pixel = fishImageRGBA.at<Vec4b>(y, x);
                        if (pixel[3] > 0) {  // Verificar se o pixel não é transparente
                            roi.at<Vec3b>(y, x) = Vec3b(pixel[0], pixel[1], pixel[2]);
                        }
                    }
                }
            }

            // Desenhar um retângulo ao redor do rosto
            rectangle(frame, faceRect, Scalar(250, 241, 122), 2);
        }

        imshow("Jogo", frame);

        // Verificar se a tecla 'Esc' foi pressionada para sair do loop
        if (waitKey(1) == 27) {
            break;
        }
    }

    destroyAllWindows();
    capture.release();

    return 0;
}
