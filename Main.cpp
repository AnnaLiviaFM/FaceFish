#include <opencv2/opencv.hpp>
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

using namespace cv;

int main()
{
    // Carregar o classificador Haar Cascade para detecção de rosto
    CascadeClassifier faceCascade;
    faceCascade.load("haarcascade_frontalface_default.xml");

    // Carregar a imagem do peixe
    Mat fishImage = imread("fish.png", IMREAD_UNCHANGED);

    // Inicializar a webcam
    VideoCapture cap(0);
    if (!cap.isOpened())
    {
        std::cout << "Erro ao abrir a câmera!" << std::endl;
        return -1;
    }

    // Loop principal do jogo
    while (true)
    {
        Mat frame;
        cap >> frame;

        // Converter a imagem para escala de cinza
        Mat grayFrame;
        cvtColor(frame, grayFrame, COLOR_BGR2GRAY);

        // Detectar rostos na imagem
        std::vector<Rect> faces;
        faceCascade.detectMultiScale(grayFrame, faces, 1.1, 3, 0 | CASCADE_SCALE_IMAGE, Size(30, 30));

        // Desenhar o peixe no centro de cada rosto detectado
        for (const Rect& face : faces)
        {
            // Calcular as coordenadas do centro do rosto
            int faceCenterX = face.x + face.width / 2;
            int faceCenterY = face.y + face.height / 2;

            // Calcular as coordenadas do canto superior esquerdo do peixe
            int fishX = faceCenterX - fishImage.cols / 2;
            int fishY = faceCenterY - fishImage.rows / 2;

            // Verificar se as coordenadas estão dentro dos limites da imagem
            if (fishX >= 0 && fishY >= 0 && fishX + fishImage.cols <= frame.cols && fishY + fishImage.rows <= frame.rows)
            {
                // Desenhar o peixe na imagem
                for (int y = 0; y < fishImage.rows; y++)
                {
                    for (int x = 0; x < fishImage.cols; x++)
                    {
                        Vec4b fishPixel = fishImage.at<Vec4b>(y, x);

                        // Se o pixel do peixe não for transparente, desenhe na imagem do jogo
                        if (fishPixel[3] > 0)
                        {
                            Vec3b& gamePixel = frame.at<Vec3b>(fishY + y, fishX + x);
                            gamePixel[0] = fishPixel[0];
                            gamePixel[1] = fishPixel[1];
                            gamePixel[2] = fishPixel[2];
                        }
                    }
                }
            }
        }

       
        // Mostrar a imagem do jogo
        imshow("Fish Game", frame);

        // Verificar se a tecla 'q' foi pressionada para sair do jogo
        if (waitKey(1) == 'q')
        {
            break;
        }
    }

    // Encerrar o jogo e liberar os recursos
    cap.release();
    destroyAllWindows();

    return 0;
}
