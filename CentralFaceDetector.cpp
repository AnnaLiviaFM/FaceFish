//Incluindo bibliotecas a serem utilizadas

#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>

using namespace std;
using namespace cv;

//chamando a função detectAndDraw

void detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale, bool tryflip, int frameWidth, int frameHeight);

string cascadeName;

int main( int argc, const char** argv ){
    VideoCapture capture;
    Mat frame;
    bool tryflip;
    CascadeClassifier cascade;
    double scale;

    cascadeName = "haarcascade_frontalface_default.xml";
    scale = 1; // usar 1, 2, 4.
    if (scale < 1)
        scale = 1;
    tryflip = true;

    if (!cascade.load(cascadeName)){
        cerr << "ERROR: Could not load classifier cascade: " << cascadeName << endl;
        return -1;
    }

    if(!capture.open(0)) // para testar com a webcam
    {
        cout << "Capture from camera #0 didn't work" << endl;
        return 1;
    }

    if( capture.isOpened() ){
        cout << "Video capturing has been started ..." << endl;
        
        // Defina o tamanho do quadro de vídeo
        int frameWidth = capture.get(cv::CAP_PROP_FRAME_WIDTH);
        int frameHeight = capture.get(cv::CAP_PROP_FRAME_HEIGHT);

        while (1)
        {
            capture >> frame;
            if( frame.empty() )
                break;

            detectAndDraw( frame, cascade, scale, tryflip, frameWidth, frameHeight );

            char c = (char)waitKey(10);
            if( c == 27 || c == 'q' || c == 'Q' )
                break;
        }
    }

    return 0;
}

/*
 * @brief Draws a transparent image over a frame Mat.
 * @param frame o frame onde a imagem trasparente será desenhada
 * @param transp the Mat image with transparency, read from a PNG image, with the IMREAD_UNCHANGED flag
 * @param xPos posição x do frame onde a imagem vai começar 
 * @param yPos posição y do frame onde a imagem vai começar 
 */
void drawTransparency(Mat frame, Mat transp, int xPos, int yPos) {
    Mat mask;
    vector<Mat> layers;

    split(transp, layers); // seperate channels
    Mat rgb[3] = { layers[0],layers[1],layers[2] };
    mask = layers[3]; // png's alpha channel used as mask
    merge(rgb, 3, transp);  // put together the RGB channels, now transp isn't transparent 
    transp.copyTo(frame.rowRange(yPos, yPos + transp.rows).colRange(xPos, xPos + transp.cols), mask);
}

/*
 * @param color the color of the rect
 * @param alpha transparence level. 0 is 100% transparent, 1 is opaque.
 * @param region rect region where the should be positioned
 */

void drawTransRect(Mat frame, Scalar color, double alpha, Rect region) {
    Mat roi = frame(region);
    Mat rectImg(roi.size(), CV_8UC3, color); 
    addWeighted(rectImg, alpha, roi, 1.0 - alpha , 0, roi); 
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade, double scale, bool tryflip, int frameWidth, int frameHeight){
    double t = 0;
    vector<Rect> faces;
    Mat gray, smallImg;
    Scalar color = Scalar(252,150,86);

    double fx = 1 / scale;
    resize( img, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT );
    if( tryflip )
        flip(smallImg, smallImg, 1);
    cvtColor( smallImg, gray, COLOR_BGR2GRAY );
    equalizeHist( gray, gray );

    t = (double)getTickCount();

    cascade.detectMultiScale( gray, faces,
        1.3, 2, 0
        //|CASCADE_FIND_BIGGEST_OBJECT
        //|CASCADE_DO_ROUGH_SEARCH
        |CASCADE_SCALE_IMAGE,
        Size(40, 40) );
    t = (double)getTickCount() - t;
    printf( "detection time = %g ms\n", t*1000/getTickFrequency());
    // PERCORRE AS FACES ENCONTRADAS
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Rect r = faces[i];
        rectangle( smallImg, Point(cvRound(r.x), cvRound(r.y)),
                    Point(cvRound((r.x + r.width-1)), cvRound((r.y + r.height-1))),
                    color, 3);
    }

    // Desenha quadrados com transparência 86, 150, 252
    double alpha = 0.5;
    drawTransRect(smallImg, Scalar(252,150,86), alpha, Rect(0, 0, frameWidth, frameHeight));


    // Desenha uma imagem
    Mat shark = cv::imread("fish.png", IMREAD_UNCHANGED);
    drawTransparency(smallImg, shark, 30, 400);
    printf("shark::width: %d, height=%d\n", shark.cols, shark.rows );

    // Desenha um texto
    color = Scalar(0, 0, 0);
    putText(smallImg, "Pontos:", Point(400, 50), FONT_HERSHEY_PLAIN, 2, color); // fonte
     putText(smallImg, "Menu:", Point(20, 300), FONT_HERSHEY_PLAIN, 2, color); // menu


    // Desenha o frame na tela
    imshow("result", smallImg );
    printf("image::width: %d, height=%d\n", smallImg.cols, smallImg.rows );
}
