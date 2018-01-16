// Traffic counting by use of opencv library
// Steve Dreger, November 2017 - January 2018
// ------------------------------------------

#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <iostream>
#include <sstream>
using namespace cv;
using namespace std;

//Konstanten:
const int CONST_VEHICLE_TYPE_BIKE = 1;      //id für typ Rad
const int CONST_VEHICLE_TYPE_CAR  = 2;      //id für typ auto
const int CONST_VEHICLE_TYPE_TRAM = 3;      //id für typ lkw/tram
const double CONST_MIN_OBJSIZE = 8000.0;    //mindestgröße für ein zu verfolgendes Objekt
const int CONST_MAX_DISTANZ = 150;          //max. Abstand zwischen zwei Positionen damit als gleiches Objekt erkannt (schneller PC evtl. heruntersetzen)
const int CONST_MIN_GROESSE_AUTO = 18000;   //min. größe damit Objekt als Auto erkannt wird
const int CONST_MIN_GROESSE_LKWTRAM = 80000;//min größe, damit Objekt als LKW/TRAM erkannt wird

//Globale Variablen
Mat frame;                          //speichert Originalbild
Mat fgMaskMOG2;                     //enthält nur noch background substrakt
Mat legende;                 // Legende am Kopf einzublenden
Mat zwischenSchrittBild;
Ptr<BackgroundSubtractor> pMOG2 = createBackgroundSubtractorMOG2();    //MOG2 Background subtractor
char keyboard   = 0;                      //input from keyboard
int thresh      = 100;                   //aktuell eingestellter Threshold-Wert, wird von Regler direkt geändert
int carCountL    = 0;       //gezählte Autos mit Richtung links
int bikeCountL   = 0;       //gezählte Räder mit Richtung links
int tramCountL   = 0;       //gezählte LKW/Trams mit Richtung links
int carCountR    = 0;        //gezählte Autos mit Richtung rechts
int bikeCountR   = 0;       //...
int tramCountR   = 0;       //...
int iBildZaehler = 0;       //Zähler Nummer angezeigtes Frame im Film
int objektZaehlerTotal = 0;
char timestamp[22];

char* getTimeString()
{
	time_t Zeitstempel = time(0); 
	tm *nun = localtime(&Zeitstempel);
	strftime(timestamp, 22, "%d.%m.%Y %H:%M:%S: ", nun);
	return timestamp;
}


//Klasse für ein verfolgtes Objekt
class BewegtesObjekt
{
    public:
    Point mittelpunkt;
    Point mittelpunkt_last;
    Rect rahmen;
    int objektIndex;
    int lastFoundInFrame;
    int typ;
    //konstruktor
    BewegtesObjekt (Point mp,  Rect boundRect, int i, int foundInFrame)
    {
        mittelpunkt.x = mittelpunkt_last.x = mp.x;
        mittelpunkt.y = mittelpunkt_last.y = mp.y;
        objektIndex = i;
        lastFoundInFrame = foundInFrame;
        rahmen = boundRect;
        typ = 0;
    }
    //setzt position neu
    void aktualisierePosition (Point p,  Rect boundRect, int foundInFrame)
    {
        mittelpunkt_last.x = mittelpunkt.x;
        mittelpunkt_last.y = mittelpunkt.y;
        mittelpunkt.x = p.x;
        mittelpunkt.y = p.y;
        lastFoundInFrame = foundInFrame;
        rahmen = boundRect;
    }
    //zeichnet rahmen und mittelpunkt sowie beschreibung
    void darstellen(Mat frame)
    {
            rectangle( frame, rahmen.tl(), rahmen.br(), Scalar(0,255,0), 2, 8, 0 );
            circle(frame, mittelpunkt, 3, Scalar(0,255,0));
            switch (typ)
            {
                case  CONST_VEHICLE_TYPE_BIKE:
                    putText(frame, "Rad", rahmen.tl(), FONT_HERSHEY_SIMPLEX, 0.70, Scalar (0,255,0));
                    break;
                case CONST_VEHICLE_TYPE_CAR:
                    putText(frame, "Auto", rahmen.tl(), FONT_HERSHEY_SIMPLEX, 0.70, Scalar (0,255,0));
                    break;
                case CONST_VEHICLE_TYPE_TRAM:
                    putText(frame, "LKW/Tram", rahmen.tl(), FONT_HERSHEY_SIMPLEX, 0.70, Scalar (0,255,0));
                    break;
            }
            if (mittelpunkt_last.x < mittelpunkt.x)
                putText(frame, "-->", Point(rahmen.tl().x+50, rahmen.tl().y-5), FONT_HERSHEY_SIMPLEX, 0.50, Scalar (0,255,0));
            else
                putText(frame, "<--", Point(rahmen.tl().x+50, rahmen.tl().y-5), FONT_HERSHEY_SIMPLEX, 0.50, Scalar (0,255,0));
    }

    //versucht objekt als rad, auto, lkw-tram zu erkennen und relevanten zähler zu erhöhen
    void klassifiziereUndZaehle(double groesse)
    {
        if (mittelpunkt.x > 200 && mittelpunkt.x < frame.cols-200)
        {
            if (groesse < CONST_MIN_GROESSE_AUTO)
            {
                typ = CONST_VEHICLE_TYPE_BIKE;
		cout << getTimeString() << "Objekt "<< objektIndex << " als Passant/Rad klassifiziert "<< " (Groesse: " << groesse << ")" << endl ;
                if (mittelpunkt_last.x < mittelpunkt.x) bikeCountR++; else bikeCountL++;
            }
            else if (groesse < CONST_MIN_GROESSE_LKWTRAM)
            {
                typ = CONST_VEHICLE_TYPE_CAR;
		cout << getTimeString() << "Objekt "<< objektIndex << " als Auto klassifiziert "<< " (Groesse: " << groesse << ")" << endl ;
                if (mittelpunkt_last.x < mittelpunkt.x) carCountR++; else carCountL++;
            }
            else
            {
                typ = CONST_VEHICLE_TYPE_TRAM;
		cout << getTimeString() << "Objekt "<< objektIndex << " als LKW/Tram klassifiziert "<< " (Groesse: " << groesse << ")" << endl ;
                if (mittelpunkt_last.x < mittelpunkt.x) tramCountR++; else tramCountL++;
            }
        
        }

    }
};
vector<BewegtesObjekt> listeBewegteObjekte; // liste aller gerade verfolgten Objekte
void mainLoop(VideoCapture capture);

int main(int argc, char* argv[])
{
    //Fenster einrichten
    namedWindow("Verkehr zaehlen ...");
    createTrackbar( " Schwellwert:", "Verkehr zaehlen ...", &thresh, 255);

    //Videoquelle öffnen
    VideoCapture capture("fe48.webm");
    //VideoCapture capture(0);
    if(capture.isOpened())
    {
        legende  = imread("legende.png", CV_LOAD_IMAGE_COLOR);
        //Hauptfenster bis Ende Video oder Taste 'q'
        mainLoop(capture);
         //Aufräumen und Ende
        capture.release();
        destroyAllWindows();
        return EXIT_SUCCESS;
    }
    else
    {
        //error in opening the video input
        cout << getTimeString() << "Kann Standard-WebCam nicht oeffnen." << endl;
        exit(EXIT_FAILURE);
    }

}
//gefundene bewegte Beriche im Bild bekannten Objekten zuprdmem oder ein neues Obj. erstellen
int findeExistierendesOderErstelleNeuesObjekt(vector<Point>  umriss, int frameCount)
{
    //rahmen/zentralen Punkt um Blob zeichnen
    vector<Point> contours_poly( umriss.size() );
    approxPolyDP( Mat(umriss), contours_poly, 3, true );
    Rect boundRect = boundingRect( Mat(contours_poly) );
    Point p;
    p.x = (boundRect.x + boundRect.x + boundRect.width) / 2;
    p.y = (boundRect.y + boundRect.y + boundRect.height) / 2;

    //Fallunterscheidung: findet sich in der Nähe des bewegten Bereiches ein Objekt der letzten Franes ?
    for (unsigned int i=0; i<listeBewegteObjekte.size(); i++)
    {
        BewegtesObjekt &bo = listeBewegteObjekte[i];
        //sofern das Objekt in den letzten 5 Bildern identifiziert wurde
        if (frameCount - bo.lastFoundInFrame < 5)
        {
            int intX = abs(p.x - bo.mittelpunkt.x);
            int intY = abs(p.y - bo.mittelpunkt.y);
            int distanz = sqrt(pow(intX, 2) + pow(intY, 2));
            if (distanz < CONST_MAX_DISTANZ)
            {
            //bekanntes Objekt
            //cout << "Objekt wiedergefunden id "<< i <<" Typ " << bo.typ <<"bisher: " << bo.mittelpunkt << " jetzt: " << p << " dist =" << distanz << ") \n";
            bo.aktualisierePosition(p, boundRect, frameCount);
            return i;
            }
        }

    }
    //oder nicht: dann folgt: neues Objekt
    BewegtesObjekt bewObj = BewegtesObjekt(p, boundRect, objektZaehlerTotal, frameCount);
    listeBewegteObjekte.push_back(bewObj);
    objektZaehlerTotal++;
    cout << getTimeString() << "Neues Objekt Id="<< objektZaehlerTotal-1 << "\n";
    return listeBewegteObjekte.size()-1;
}

//löscht aus globaler Liste alle Objekte, die Bild verlassen haben - Performance-Tuning
void entferneUnsichtbarGewordeneObjekte(int frameCount)
{
    unsigned int i=0;
    while (i < listeBewegteObjekte.size())
    {
        BewegtesObjekt &bo = listeBewegteObjekte[i];
        //sofern das Objekt in den letzten 5 Bildern identifiziert wurde
        if (frameCount - bo.lastFoundInFrame >= 5)
        {
            cout << getTimeString() << "Entferne Objekt Id=" << bo.objektIndex << " (Listenpos. " << i <<") aus Tracking-Liste, da außerhalb Bild.\n";
            listeBewegteObjekte.erase(listeBewegteObjekte.begin()+i);
        }
        else
        {
            i++;
        }
    }
}

// bild für bild den Videoeingang analysieren, Objekte identifzieren und zählen bis 'q'
void mainLoop(VideoCapture capture) {
    while( keyboard != 'q' && keyboard != 27 )
    {
        // Zähler zurücksetzen
        if (keyboard == 'r')
        {
            carCountL = carCountR = 0;
            bikeCountL = bikeCountR = 0;
            tramCountL = tramCountR = 0;
        }

        //read the current frame
        if(!capture.read(frame)) {
            cerr << "Unable to read next frame." << endl;
            cerr << "Exiting..." << endl;
            exit(EXIT_FAILURE);
        }

        //Bild bearbeiten: NachGrau, Weichzeichnen, Hintergrund-rausrechnen, Schwellwert kleine Löcher füllen / glätten mit Weiß
        Size vorschauGroesse = Size(128,96);
        cvtColor(frame, fgMaskMOG2, CV_BGR2GRAY);
            resize (fgMaskMOG2, zwischenSchrittBild, vorschauGroesse); imshow("1 Grau", zwischenSchrittBild);
        GaussianBlur(fgMaskMOG2, fgMaskMOG2, Size(21, 21), 0);
            resize (fgMaskMOG2, zwischenSchrittBild, vorschauGroesse); imshow("2 Weichzeichnen", zwischenSchrittBild);
        pMOG2->apply(fgMaskMOG2, fgMaskMOG2);
            resize (fgMaskMOG2, zwischenSchrittBild, vorschauGroesse); imshow("3 Nur Bewegung", zwischenSchrittBild);
        threshold(fgMaskMOG2, fgMaskMOG2, thresh, 255.0, CV_THRESH_BINARY);
        Mat structuringElement5x5 = getStructuringElement(MORPH_RECT, Size(10, 10));
        dilate(fgMaskMOG2,fgMaskMOG2,structuringElement5x5,Point(-1,-1),2);
            resize (fgMaskMOG2, zwischenSchrittBild, vorschauGroesse); imshow("4 Ohne Rauschen", zwischenSchrittBild);

        //hole liste der Umrisse aller noch existierenden bewegten Bereiche
        vector<vector<Point> > contours;
        findContours(fgMaskMOG2, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

        //alle Bewegungsbereiche im Bild hintereinander auf Mindestgröße und dann Objektzuordnung checken
        for (unsigned int i = 0; i < contours.size(); i++)
        {
            double contSize = contourArea(contours[i]);

            //relevante Bereiche haben Mindestgröße ...
            if (contSize > CONST_MIN_OBJSIZE)
            {
                //relevanter Blob, wird in Liste der im aktuellen Fram erkannten Blobs aufgenommen
                int indexBewegtesObjekt = findeExistierendesOderErstelleNeuesObjekt(contours[i], iBildZaehler);
                BewegtesObjekt &bewObj = listeBewegteObjekte[indexBewegtesObjekt];
                if (!bewObj.typ) bewObj.klassifiziereUndZaehle(contSize);
                bewObj.darstellen(frame);
            }

        }

        // Anzeige: Bild mit markierten Objekten und Zähler anzeigen
        stringstream vehicleCountDisplay;
        vehicleCountDisplay << "     " << bikeCountL <<"|" << bikeCountR << "         " << carCountL <<"|" << carCountR  << "              " << tramCountL-1 <<"|" << tramCountR;
        legende.copyTo(frame(Rect(0, 0, legende.cols, legende.rows)));
        putText(frame, vehicleCountDisplay.str(), Point (10,18), FONT_HERSHEY_SIMPLEX, 0.6, Scalar (0,0,0), 1);
        imshow("Verkehr zaehlen ...", frame);
        //Tastatureingabe prüfen
        keyboard = (char)waitKey( 30 );
        //alle 100 Bilder aufräumen
        if (iBildZaehler%100==0) entferneUnsichtbarGewordeneObjekte(iBildZaehler);
        iBildZaehler++;
    }
}
