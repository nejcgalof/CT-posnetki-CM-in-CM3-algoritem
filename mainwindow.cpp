#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtGui>
#include <QLabel>
#include <QFileDialog>
#include <QString>
#include <string>
#include <fstream>
#include <istream>
#include <iostream>
#include <bitset>
#include <cmath>
#include <QThread>
#include <sstream>
#include <QMessageBox>


using namespace std;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QVector<signed short> vet(524288);
QVector<byte> barva(768);
int prva;
int druga;
int N=512;
void MainWindow::on_nalozi_sliko_triggered()
{
    N=ui->lineEdit->text().toInt();
    QString imedat = QFileDialog::getOpenFileName(this,tr("Izberi"),"",tr("IMG datoteka (*.img *.jpg)"));
    if(QString::compare(imedat,QString())!=0){//ce sem izbral pot
        QByteArray fileNameArr = QFile::encodeName(imedat);
        const char * fileNameStr = fileNameArr.constData();//v tekstu pot
        //branje slike
        QImage img(N, N, QImage::Format_RGB888);//ustvarim sliko
        img.fill(QColor(Qt::white).rgb());//z belo podlago
        ifstream file(fileNameStr, ios::in | ios::binary);//odprem datoteko-binarno
        file.seekg (0, ios::end);//preverjam velikost - vse datoteke enako velike
        int tam = file.tellg();
        prva=tam;
        file.seekg (0, ios::beg);
        qint64 ns = tam/sizeof(signed short);//stevilo signed shortov
        vet.clear();
        vet.resize(N*N*2);
        for(int i = 0; i < tam; i++){//berem po signed shortih do konca
            file.read(reinterpret_cast <char * > (&vet[i]), sizeof(signed short));//polnim v vector
        }
        file.close();//zaprem
        int k=0;
        //ker je 512x512
        for(int i=0;i<N;i++){
            for(int j=0;j<N;j++){
               // cout<<"vet:"<<vet[k]<<"\n";
                //hitri prikaz brez palet v rdeci barvi
                int pix =(int)((((float)vet[k]+2048.f)/4095.f)*255.f);
                img.setPixel(i,j,qRgb(pix, 0, 0));
                k+=1;
            }
        }
       //prikaz
       ui->label->setPixmap(QPixmap::fromImage(img));
       ui->label->show();
    }
}

void MainWindow::on_nalozi_paleto_triggered()
{
    QString imedat = QFileDialog::getOpenFileName(this,tr("Izberi"),"",tr("LUT datoteka (*.lut)"));
    if(QString::compare(imedat,QString())!=0){//ce gre odpreti
        QByteArray fileNameArr = QFile::encodeName(imedat);
        const char * fileNameStr = fileNameArr.constData();//dobimo pot v tekstu
        //branje
        ifstream file1(fileNameStr, ios::in | ios::binary);//odpremo binarno
        file1.seekg (0, ios::end);//parametri za zacetek in konec
        int tam = file1.tellg();
        file1.seekg (0, ios::beg);
        qint64 ns = tam/sizeof(byte);//koncno stevilo
        barva.clear();
        barva.resize(768);
        for(int i = 0; i < tam; i++){
            file1.read(reinterpret_cast <char * > (&barva[i]), sizeof(byte));//berem byte po byte in vstavljam
        }
        file1.close();//zaprem
        QImage img(N, N, QImage::Format_RGB888);//slika velika 512x512
        img.fill(QColor(Qt::white).rgb());//bela podlaga
        int k=0;
        //cout<<vet.size();
        int k_st=0;
        for(int i=0;i<N;i++){//vrstice
            for(int j=0;j<N;j++){//stolpci
              //  cout<<vrednosti[k]<<" ";
                    int indeks =(int)((((float)vet[k]+2048.f)/4095.f)*255.f);//dobim indeks
                    //cout<<vet[k]<<"\n";
                    //preberem rgb vrednosti
                    img.setPixel(i,j,qRgb((int)barva[(indeks*3)],(int)barva[(indeks*3)+1],(int)barva[(indeks*3)+2]));

                k+=1;//pomaknem na naslednji indeks
            }
        }//konec for-a
       // cout<<"\nkokrat ni ok:"<<k_st;
        //prikazem
        ui->label->setPixmap(QPixmap::fromImage(img));
        ui->label->show();
    }
}
vector<char> telo;
int st_ponovitve=0;
int absrazlika=0;
int razlika=0;
string koda="";
void MainWindow::CM_kodiraj(){
    N=ui->lineEdit->text().toInt();
    koda="";
    //za testiranje
   /* vet.clear();
    vet.insert(vet.end(),-2048);
    vet.insert(vet.end(),-2048);
    vet.insert(vet.end(),-2048);
    vet.insert(vet.end(),55);
    vet.insert(vet.end(),53);
     vet.insert(vet.end(),53);
      vet.insert(vet.end(),53);
       vet.insert(vet.end(),53);
       vet.insert(vet.end(),51);
       vet.insert(vet.end(),51);
       vet.insert(vet.end(),51);
       vet.insert(vet.end(),10);
       vet.insert(vet.end(),-11);
       vet.insert(vet.end(),-11);
       vet.insert(vet.end(),-11);
       vet.insert(vet.end(),-2048);*/
    //zapisi s stringi
    int i=0;
    while(i<vet.size()){//grem cez vse elemente
        if(i>=N*N){
            break;
        }
        if(i==0){
            //za prvega pogoji (samo zrak ali absolutna je lahko
            if(vet[i]==(signed short)-2048){//ce je zrak
                koda+="11";
            }
            else{//je absolutna
                koda+="10";
                bitset<12> vred1(vet[i]);
                koda+=vred1.to_string();
            }
           i++;
        }//konec za prvega
        else{
            absrazlika=(int)abs(vet[i]-vet[i-1]);
            //cout<<absrazlika<<"\n";

            if(vet[i]==(signed short)-2048){//ce je zrak
                 koda+="11";
                 i++;
             }
           else if(absrazlika<=30&&absrazlika>0){//ce je v tem intervalu obdelam razliko
            koda+="00"; //razlika
            razlika=(int)(vet[i]-vet[i-1]);

            if(absrazlika<=2){
                koda+="00";//2 bita
                if(razlika==-2){koda+="00";}
                else if(razlika==-1){koda+="01";}
                else if(razlika==1){koda+="10";}
                else if(razlika==2){koda+="11";}
            }
            else if(absrazlika>=3&&absrazlika<=6){//med 3 in 6
                koda+="01";//3 biti
                if(razlika==-6){koda+="000";}
                else if(razlika==-5){koda+="001";}
                else if(razlika==-4){koda+="010";}
                else if(razlika==-3){koda+="011";}
                else if(razlika==3){koda+="100";}
                else if(razlika==4){koda+="101";}
                else if(razlika==5){koda+="110";}
                else if(razlika==6){koda+="111";}
            }
            else if(absrazlika>=7&&absrazlika<=14){
                koda+="10";//4 biti
                if(razlika==-14){koda+="0000";}
                else if(razlika==-13){koda+="0001";}
                else if(razlika==-12){koda+="0010";}
                else if(razlika==-11){koda+="0011";}
                else if(razlika==-10){koda+="0100";}
                else if(razlika==-9){koda+="0101";}
                else if(razlika==-8){koda+="0110";}
                else if(razlika==-7){koda+="0111";}
                else if(razlika==7){koda+="1000";}
                else if(razlika==8){koda+="1001";}
                else if(razlika==9){koda+="1010";}
                else if(razlika==10){koda+="1011";}
                else if(razlika==11){koda+="1100";}
                else if(razlika==12){koda+="1101";}
                else if(razlika==13){koda+="1110";}
                else if(razlika==14){koda+="1111";}
            }
            else if(absrazlika>=15&&absrazlika<=30){
                koda+="11";//5 bitov
                if(razlika==-30){koda+="00000";}
                else if(razlika==-29){koda+="00001";}
                else if(razlika==-28){koda+="00010";}
                else if(razlika==-27){koda+="00011";}
                else if(razlika==-26){koda+="00100";}
                else if(razlika==-25){koda+="00101";}
                else if(razlika==-24){koda+="00110";}
                else if(razlika==-23){koda+="00111";}
                else if(razlika==-22){koda+="01000";}
                else if(razlika==-21){koda+="01001";}
                else if(razlika==-20){koda+="01010";}
                else if(razlika==-19){koda+="01011";}
                else if(razlika==-18){koda+="01100";}
                else if(razlika==-17){koda+="01101";}
                else if(razlika==-16){koda+="01110";}
                else if(razlika==-15){koda+="01111";}
                else if(razlika==15){koda+="10000";}
                else if(razlika==16){koda+="10001";}
                else if(razlika==17){koda+="10010";}
                else if(razlika==18){koda+="10011";}
                else if(razlika==19){koda+="10100";}
                else if(razlika==20){koda+="10101";}
                else if(razlika==21){koda+="10110";}
                else if(razlika==22){koda+="10111";}
                else if(razlika==23){koda+="11000";}
                else if(razlika==24){koda+="11001";}
                else if(razlika==25){koda+="11010";}
                else if(razlika==26){koda+="11011";}
                else if(razlika==27){koda+="11100";}
                else if(razlika==28){koda+="11101";}
                else if(razlika==29){koda+="11110";}
                else if(razlika==30){koda+="11111";}
            }
            i++;//grem na naslednji element
           }//ce ni razlika do 30
           else if(vet[i]==vet[i-1]){//ce je ponovitev?
               bool enako=true;
               int k=i;
               st_ponovitve=0;
                while(enako){
                    if(vet[k]==vet[i-1]){//ce je k-ti element enak prejsnemu povecamo stevec
                        st_ponovitve++;
                        k++;
                    }
                    else{
                        enako=false;
                    }
                    if(st_ponovitve>63){//tudi tukaj koncamo
                        enako=false;
                    }
                    if(k>=vet.size()){//tudi tukaj koncamo
                        enako=false;
                    }
                }//preneham steti razlike
                //vnesem
                koda+="01";//ponovitve
                bitset<6> vred2(st_ponovitve);
                koda+=vred2.to_string();
                i+=st_ponovitve;//toliko pomaknem naprej
           }
           else{//absolutno
                koda+="10";
                bitset<12> vred3(vet[i]);
               // cout<<vet[i]<<"\n";
                koda+=vred3.to_string();
                i++;
           }
        }
    }//konec
}
void MainWindow::on_actionCM_triggered()
{
    CM_kodiraj();
    string_v_vector();//iz stringa v binarni zapis-bit shifting
    ofstream outfile ("new.nejc",ofstream::binary);
    outfile.write(reinterpret_cast<const char*>(&telo[0]), telo.size()*sizeof(char));//zapisem vektor
    outfile.close();
    //primerjava
    QMessageBox Msgbox;
            cout<<"Originalna: "<<prva<<"B\n";
            ifstream infile2 ("new.nejc", ifstream::ate | ifstream::binary|ios::in);
            druga=infile2.tellg(); //dobim velikost v bytih cele datoteke
            cout<<"Stisjena:   "<<druga<<"B\n";
            double procenti =100- (druga*100)/prva;
            if(procenti<0){
                procenti=0;
            }
            cout<<"Stisjeno za "<<procenti<<" %\n";
            infile2.close();
            QString text = QString("Originalna:" + QString::number(prva)+"B\nStisnjenja: "+QString::number(druga)+"B\nStisnjeno za: "+QString::number(procenti)+"%\n");
            Msgbox.setText(text);
            Msgbox.setStandardButtons(QMessageBox::Ok);
            Msgbox.exec();
}

struct C {
    signed short b : 12; // 12 bit integer
};
struct C2{
    signed short b : 2; // 12 bit integer
};
struct C4{
    signed short b : 4; // 12 bit integer
};
struct C8{
    signed short b : 8; // 12 bit integer
};
void MainWindow::on_actionCM_Dekodiraj_triggered()
{
    QString imedat = QFileDialog::getOpenFileName(this,tr("Izberi"),"",tr("NEJC datoteka (*.nejc)"));
    if(QString::compare(imedat,QString())!=0){//ce sem izbral pot
        QByteArray fileNameArr = QFile::encodeName(imedat);
        const char * fileNameStr = fileNameArr.constData();//v tekstu pot
        ifstream infile(fileNameStr, ios::in | ios::binary);//odprem datoteko-binarno
        char c; //bere znak
        string koda="";
        int index=0;
       // vet.clear();
        vet.clear();
        while(infile.get(c))//berem do konca - pridobivam byte
        {
            for(int i=0;i<8;i++){//za vsak bit v bytu
                char bit= c & 0x80; //da dobim prvega 1000000
                c<<=1; //premaknem v levo za 1
                if(bit==0){//ce je 0
                    koda+="0";
                }
                else{//ce je 1
                    koda+="1";
                }
            }
        }
            //cout<<koda<<"\n";
            //sedaj imam kodo
            string prva="";//prva dva bita
            string druga="";//druga dva bita
            string tretji="";//tretji biti
            int stevka=0;
            while(index<koda.length()){//pridem do konca
                prva=koda.substr(index,2);//vzamem 2
                index+=2;//pomaknem naprej
                if(prva=="11"){//ce je zrak
                    vet.insert(vet.end(),(signed short)-2048);
                    stevka++;
                    if(stevka>N*N){break;}
                }
                else if(prva=="00"){//ce je razlika
                    druga=koda.substr(index,2);//vzamem naslednja 2
                    index+=2;//pomaknem naprej
                    if(druga=="00"){//2 bita
                        tretji=koda.substr(index,2);//vzamem naslednja 2
                        index+=2;//pomaknem naprej
                        if(tretji=="00"){vet.insert(vet.end(),vet.back()-2);}
                        else if(tretji=="01"){vet.insert(vet.end(),vet.back()-1);}
                        else if(tretji=="10"){vet.insert(vet.end(),vet.back()+1);}
                        else if(tretji=="11"){vet.insert(vet.end(),vet.back()+2);}
                    }
                    else if(druga=="01"){//3 biti
                        tretji=koda.substr(index,3);//vzamem naslednjih 3
                        index+=3;//pomaknem naprej
                        if(tretji=="000"){vet.insert(vet.end(),vet.back()-6);}
                        else if(tretji=="001"){vet.insert(vet.end(),vet.back()-5);}
                        else if(tretji=="010"){vet.insert(vet.end(),vet.back()-4);}
                        else if(tretji=="011"){vet.insert(vet.end(),vet.back()-3);}
                        else if(tretji=="100"){vet.insert(vet.end(),vet.back()+3);}
                        else if(tretji=="101"){vet.insert(vet.end(),vet.back()+4);}
                        else if(tretji=="110"){vet.insert(vet.end(),vet.back()+5);}
                        else if(tretji=="111"){vet.insert(vet.end(),vet.back()+6);}
                    }
                    else if(druga=="10"){//4 biti
                        tretji=koda.substr(index,4);//vzamem naslednjih 4
                        index+=4;//pomaknem naprej
                        if(tretji=="0000"){vet.insert(vet.end(),vet.back()-14);}
                        else if(tretji=="0001"){vet.insert(vet.end(),vet.back()-13);}
                        else if(tretji=="0010"){vet.insert(vet.end(),vet.back()-12);}
                        else if(tretji=="0011"){vet.insert(vet.end(),vet.back()-11);}
                        else if(tretji=="0100"){vet.insert(vet.end(),vet.back()-10);}
                        else if(tretji=="0101"){vet.insert(vet.end(),vet.back()-9);}
                        else if(tretji=="0110"){vet.insert(vet.end(),vet.back()-8);}
                        else if(tretji=="0111"){vet.insert(vet.end(),vet.back()-7);}
                        else if(tretji=="1000"){vet.insert(vet.end(),vet.back()+7);}
                        else if(tretji=="1001"){vet.insert(vet.end(),vet.back()+8);}
                        else if(tretji=="1010"){vet.insert(vet.end(),vet.back()+9);}
                        else if(tretji=="1011"){vet.insert(vet.end(),vet.back()+10);}
                        else if(tretji=="1100"){vet.insert(vet.end(),vet.back()+11);}
                        else if(tretji=="1101"){vet.insert(vet.end(),vet.back()+12);}
                        else if(tretji=="1110"){vet.insert(vet.end(),vet.back()+13);}
                        else if(tretji=="1111"){vet.insert(vet.end(),vet.back()+14);}
                    }
                    else if(druga=="11"){//5 bitov
                        tretji=koda.substr(index,5);//vzamem naslednjih 5
                        index+=5;//pomaknem naprej
                        if(tretji=="00000"){vet.insert(vet.end(),vet.back()-30);}
                        else if(tretji=="00001"){vet.insert(vet.end(),vet.back()-29);}
                        else if(tretji=="00010"){vet.insert(vet.end(),vet.back()-28);}
                        else if(tretji=="00011"){vet.insert(vet.end(),vet.back()-27);}
                        else if(tretji=="00100"){vet.insert(vet.end(),vet.back()-26);}
                        else if(tretji=="00101"){vet.insert(vet.end(),vet.back()-25);}
                        else if(tretji=="00110"){vet.insert(vet.end(),vet.back()-24);}
                        else if(tretji=="00111"){vet.insert(vet.end(),vet.back()-23);}
                        else if(tretji=="01000"){vet.insert(vet.end(),vet.back()-22);}
                        else if(tretji=="01001"){vet.insert(vet.end(),vet.back()-21);}
                        else if(tretji=="01010"){vet.insert(vet.end(),vet.back()-20);}
                        else if(tretji=="01011"){vet.insert(vet.end(),vet.back()-19);}
                        else if(tretji=="01100"){vet.insert(vet.end(),vet.back()-18);}
                        else if(tretji=="01101"){vet.insert(vet.end(),vet.back()-17);}
                        else if(tretji=="01110"){vet.insert(vet.end(),vet.back()-16);}
                        else if(tretji=="01111"){vet.insert(vet.end(),vet.back()-15);}
                        else if(tretji=="10000"){vet.insert(vet.end(),vet.back()+15);}
                        else if(tretji=="10001"){vet.insert(vet.end(),vet.back()+16);}
                        else if(tretji=="10010"){vet.insert(vet.end(),vet.back()+17);}
                        else if(tretji=="10011"){vet.insert(vet.end(),vet.back()+18);}
                        else if(tretji=="10100"){vet.insert(vet.end(),vet.back()+19);}
                        else if(tretji=="10101"){vet.insert(vet.end(),vet.back()+20);}
                        else if(tretji=="10110"){vet.insert(vet.end(),vet.back()+21);}
                        else if(tretji=="10111"){vet.insert(vet.end(),vet.back()+22);}
                        else if(tretji=="11000"){vet.insert(vet.end(),vet.back()+23);}
                        else if(tretji=="11001"){vet.insert(vet.end(),vet.back()+24);}
                        else if(tretji=="11010"){vet.insert(vet.end(),vet.back()+25);}
                        else if(tretji=="11011"){vet.insert(vet.end(),vet.back()+26);}
                        else if(tretji=="11100"){vet.insert(vet.end(),vet.back()+27);}
                        else if(tretji=="11101"){vet.insert(vet.end(),vet.back()+28);}
                        else if(tretji=="11110"){vet.insert(vet.end(),vet.back()+29);}
                        else if(tretji=="11111"){vet.insert(vet.end(),vet.back()+30);}
                    }//konec 5 bitov
                    stevka++;
                    if(stevka>N*N){break;}
                }//konec razlike
                else if(prva=="01"){//ponovitve
                    druga=koda.substr(index,6);//vzamem naslednja 2
                    index+=6;//pomaknem naprej
                    bitset<6> b(druga);//spravim v decimalno
                    int stevec=(int)b.to_ulong();
                    for(int i=0;i<stevec;i++){
                       vet.insert(vet.end(),vet.back());//dodam zadnjega na konec
                       stevka++;
                       if(stevka>N*N){break;}
                    }
                }
                else if(prva=="10"){//absolutno
                    druga=koda.substr(index,12);//vzamem naslednja 2
                    index+=12;//pomaknem naprej
                    bitset<12> b(druga);//spravim v decimalno
                    struct C c = { static_cast<signed short>(b.to_ulong()) };
                    //cout<<(signed short)b.to_ulong()<<"\n";
                    vet.insert(vet.end(),c.b);//vstavim stevilko
                    stevka++;
                    if(stevka>N*N){break;}
                }
            }//konec while-a
           /* for(int i=0;i<vet.size();i++){
                cout<<vet[i]<<"\n";
            }*/
        //}//konec while-a
        QImage img(N, N, QImage::Format_RGB888);//ustvarim sliko
        img.fill(QColor(Qt::white).rgb());//z belo podlago
        int k=0;
        for(int i=0;i<N;i++){
            for(int j=0;j<N;j++){
               // cout<<"vet:"<<vet[k]<<"\n";
                //hitri prikaz brez palet v rdeci barvi
                int pix =(int)((((float)vet[k]+2048.f)/4095.f)*255.f);
                img.setPixel(i,j,qRgb(pix, 0, 0));
                k+=1;
            }
        }
       //prikaz
       ui->label->setPixmap(QPixmap::fromImage(img));
       ui->label->show();
       ofstream outfile ("new.img",std::ios_base::binary);
       outfile.write((char*)(&vet[0]), N*N * sizeof(vet[0])) ;
       outfile.close();

    }//konec vsega
}
QVector<signed short> vet_prej(524288);
QVector<signed short> vet_zdej(524288);
vector<char> glava;

void MainWindow::on_actionCM3_triggered()
{
    prva=0;
//pisanje glave
    ofstream outfile ("cm3.cmp3",std::ios_base::binary);
    QStringList ls = QFileDialog::getOpenFileNames(this,"Izberi eno ali vec datotek", "","IMG(*.img)");
    string glava_str="";
    glava.clear();
    int lokacija=8;
    unsigned char byte = 8;
    N=ui->lineEdit->text().toInt();
    //16bitov = 2 byte-a
    bitset<16> loc(N);//locljivost
    glava_str+=loc.to_string();
    bitset<16> st_r(ls.size());//stevilo plasti
    glava_str+=st_r.to_string();
    for(int i = 0; i < ls.size(); i++) {//dobim vsa stevila
        QByteArray fileNameArr = QFile::encodeName(ls[i]);
        const char * fileNameStr = fileNameArr.constData();//v tekstu pot
        QFileInfo fi(fileNameStr);
        QString name = fi.fileName();
        QStringList pieces = name.split( "." );//da dobim samo ime
        QString neededWord = pieces.value( pieces.length() - 2 );
        bitset<16> stevil(neededWord.toShort());
        glava_str+=stevil.to_string();//dodajam stevilke
    }
    for(int i=0;i<glava_str.length();i++){
                if(lokacija==0){//resitiram in zapisem
                    glava.push_back(byte);
                    byte=0;
                    lokacija=8;
                }
                //shiftanje
                byte<<=1; //za 1 premaknem
                if(glava_str[i]=='1'){
                    byte|=1;
                }
                else if(glava_str[i]=='0'){
                    byte|=0;
                }
                //byte|=(unsigned char)koda[i]; //OR operator -zdruzim
                lokacija=lokacija-1;
    }
    glava.push_back(byte);//se zadnji dodam ko je ostal
    //zapisem vektorje
    outfile.write(reinterpret_cast<const char*>(&glava[0]), glava.size()*sizeof(char));//zapisem vektor
//Konec pisanja glave
//prebiranje
    for(int i = 0; i < ls.size(); i++) {
        QByteArray fileNameArr = QFile::encodeName(ls[i]);
        const char * fileNameStr = fileNameArr.constData();//v tekstu pot

        //branje slike
        QImage img(N, N, QImage::Format_RGB888);//ustvarim sliko
        img.fill(QColor(Qt::white).rgb());//z belo podlago
        ifstream file(fileNameStr, ios::in | ios::binary);//odprem datoteko-binarno
        file.seekg (0, ios::end);//preverjam velikost - vse datoteke enako velike
        int tam = file.tellg();
        prva+=tam;
        file.seekg (0, ios::beg);
        qint64 ns = tam/sizeof(signed short);//stevilo signed shortov
        vet.clear();
        vet.resize(N*N*2);
        for(int m = 0; m < tam; m++){//berem po signed shortih do konca
            file.read(reinterpret_cast <char * > (&vet[m]), sizeof(signed short));//polnim v vector
        }

        int k=0;
        //ker je 512x512
        for(int z=0;z<N;z++){
            for(int j=0;j<N;j++){
               // cout<<"vet:"<<vet[k]<<"\n";
                //hitri prikaz brez palet v rdeci barvi
                int pix =(int)((((float)vet[k]+2048.f)/4095.f)*255.f);
                img.setPixel(z,j,qRgb(pix, 0, 0));
                k+=1;
            }
        }
       //prikaz
       ui->label->setPixmap(QPixmap::fromImage(img));
       ui->label->show();
       qApp->processEvents();//sprocesira
        koda+="";
         //pridobim kodo
        if(i==0){//ce je prva plast pretvorimo po CM- dobim kodo
            cout<<"fajl"<<i<<"\n";
           // vet=vet_zdej;
           // vet.swap(vet_zdej);
            CM_kodiraj();
            vet_prej.clear();
            vet_prej.resize(N*N*2);
            vet_prej=vet;
        }
        else{
            //Di=Mx-(Mx-1)
            razlika_dveh_plasti();
            CM3_kodiraj();
            vet_prej.clear();
            vet_prej.resize(N*N*2);
            vet_prej=vet;
        }
        //nastavim trenutno na prejsno

        //vet_prej.swap(vet_zdej);
    }//konec branja vsake datoteke
    //celotno kodo zapisem
    string_v_vector();//iz stringa v binarni zapis-bit shifting
    outfile.write(reinterpret_cast<const char*>(&telo[0]), telo.size()*sizeof(char));//zapisem vektor
    outfile.close();
    QMessageBox Msgbox;
    //primerjava
            cout<<"Originalna: "<<prva<<"B\n";
            ifstream infile2 ("cm3.cmp3", ifstream::ate | ifstream::binary|ios::in);
            druga=infile2.tellg(); //dobim velikost v bytih cele datoteke
            cout<<"Stisjena:   "<<druga<<"B\n";
            double procenti =100- (druga*100)/prva;
            if(procenti<0){
                procenti=0;
            }
            cout<<"Stisjeno za "<<procenti<<" %\n";
            infile2.close();
     QString text = QString("Originalna:" + QString::number(prva)+"B\nStisnjenja: "+QString::number(druga)+"B\nStisnjeno za: "+QString::number(procenti)+"%\n");
     Msgbox.setText(text);
     Msgbox.setStandardButtons(QMessageBox::Ok);
     Msgbox.exec();
}
void MainWindow::CM3_kodiraj(){
    int i=0;
    N=ui->lineEdit->text().toInt();
    while(i<vet_zdej.size()){//grem cez vse elemente
       // cout<<"N:"<<N;
        if(i>=N*N){
            break;
        }
        if(i==0){
            //za prvega pogoji (samo zrak ali absolutna je lahko
            //NIČLA
            if(vet_zdej[i]==(signed short)0){
                koda+="11";
                i++;
            }
            //ABSOLUTNO
            else{//absolutno
                koda+="10";
                int absolut=(int)abs(vet_zdej[i]);
                if(absolut<=4 && absolut>0){
                    koda+="00";//3 biti
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<2> vred3(absolut-1);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<2> vred3(absolut-1);
                        koda+=vred3.to_string();
                    }
                }
                else if(absolut<=20 && absolut>=5){
                    koda+="01";//5 bitov
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<4> vred3(absolut-5);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<4> vred3(absolut-5);
                        koda+=vred3.to_string();
                    }
                }
                else if(absolut<=276 && absolut>=21){
                    koda+="10";//9 bitov
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<8> vred3(absolut-21);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<8> vred3(absolut-21);
                        koda+=vred3.to_string();
                    }
                }
                else if(absolut<=4095 && absolut>=277){
                    koda+="11";//13 bitov
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<12> vred3(absolut-277);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<12> vred3(absolut-277);
                        koda+=vred3.to_string();
                    }
                }
                i++;
            }//konec absolutno
        }//konec za prvega
        else{
            absrazlika=(int)abs(vet_zdej[i]-vet_zdej[i-1]);
            //NIČLE
            if(vet_zdej[i]==(signed short)0){
                koda+="11";
                i++;
            }
            //RAZLIKA
            else if(absrazlika<=30&&absrazlika>0){//ce je v tem intervalu obdelam razliko
                koda+="00"; //razlika
                razlika=(int)(vet_zdej[i]-vet_zdej[i-1]);
                if(absrazlika<=2){
                    koda+="00";//2 bita
                    if(razlika==-2){koda+="00";}
                    else if(razlika==-1){koda+="01";}
                    else if(razlika==1){koda+="10";}
                    else if(razlika==2){koda+="11";}
                }
                else if(absrazlika>=3&&absrazlika<=6){//med 3 in 6
                    koda+="01";//3 biti
                    if(razlika==-6){koda+="000";}
                    else if(razlika==-5){koda+="001";}
                    else if(razlika==-4){koda+="010";}
                    else if(razlika==-3){koda+="011";}
                    else if(razlika==3){koda+="100";}
                    else if(razlika==4){koda+="101";}
                    else if(razlika==5){koda+="110";}
                    else if(razlika==6){koda+="111";}
                }
                else if(absrazlika>=7&&absrazlika<=14){
                    koda+="10";//4 biti
                    if(razlika==-14){koda+="0000";}
                    else if(razlika==-13){koda+="0001";}
                    else if(razlika==-12){koda+="0010";}
                    else if(razlika==-11){koda+="0011";}
                    else if(razlika==-10){koda+="0100";}
                    else if(razlika==-9){koda+="0101";}
                    else if(razlika==-8){koda+="0110";}
                    else if(razlika==-7){koda+="0111";}
                    else if(razlika==7){koda+="1000";}
                    else if(razlika==8){koda+="1001";}
                    else if(razlika==9){koda+="1010";}
                    else if(razlika==10){koda+="1011";}
                    else if(razlika==11){koda+="1100";}
                    else if(razlika==12){koda+="1101";}
                    else if(razlika==13){koda+="1110";}
                    else if(razlika==14){koda+="1111";}
                }
                else if(absrazlika>=15&&absrazlika<=30){
                    koda+="11";//5 bitov
                    if(razlika==-30){koda+="00000";}
                    else if(razlika==-29){koda+="00001";}
                    else if(razlika==-28){koda+="00010";}
                    else if(razlika==-27){koda+="00011";}
                    else if(razlika==-26){koda+="00100";}
                    else if(razlika==-25){koda+="00101";}
                    else if(razlika==-24){koda+="00110";}
                    else if(razlika==-23){koda+="00111";}
                    else if(razlika==-22){koda+="01000";}
                    else if(razlika==-21){koda+="01001";}
                    else if(razlika==-20){koda+="01010";}
                    else if(razlika==-19){koda+="01011";}
                    else if(razlika==-18){koda+="01100";}
                    else if(razlika==-17){koda+="01101";}
                    else if(razlika==-16){koda+="01110";}
                    else if(razlika==-15){koda+="01111";}
                    else if(razlika==15){koda+="10000";}
                    else if(razlika==16){koda+="10001";}
                    else if(razlika==17){koda+="10010";}
                    else if(razlika==18){koda+="10011";}
                    else if(razlika==19){koda+="10100";}
                    else if(razlika==20){koda+="10101";}
                    else if(razlika==21){koda+="10110";}
                    else if(razlika==22){koda+="10111";}
                    else if(razlika==23){koda+="11000";}
                    else if(razlika==24){koda+="11001";}
                    else if(razlika==25){koda+="11010";}
                    else if(razlika==26){koda+="11011";}
                    else if(razlika==27){koda+="11100";}
                    else if(razlika==28){koda+="11101";}
                    else if(razlika==29){koda+="11110";}
                    else if(razlika==30){koda+="11111";}
                }
                i++;//grem na naslednji element
            }//ce ni razlika do 30
            //PONOVITVE
            else if(vet_zdej[i]==vet_zdej[i-1]){//ce je ponovitev?
                bool enako=true;
                int k=i;
                st_ponovitve=0;
                while(enako){
                    if(vet_zdej[k]==vet_zdej[i-1]){//ce je k-ti element enak prejsnemu povecamo stevec
                        st_ponovitve++;
                        k++;
                    }
                    else{
                        enako=false;
                    }
                    if(st_ponovitve>63){//tudi tukaj koncamo
                        enako=false;
                    }
                    if(k>=vet_zdej.size()){//tudi tukaj koncamo
                        enako=false;
                    }
                    if(k>=N*N){
                        enako=false;
                    }
                }//preneham steti razlike
                //vnesem
                koda+="01";//ponovitve
                bitset<6> vred2(st_ponovitve);
                koda+=vred2.to_string();
                i+=st_ponovitve;//toliko pomaknem naprej
            }
            //ABSOLUTNO
            else{//absolutno
                koda+="10";
                int absolut=(int)abs(vet_zdej[i]);
                if(absolut<=4 && absolut>0){
                    koda+="00";//3 biti
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<2> vred3(absolut-1);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<2> vred3(absolut-1);
                        koda+=vred3.to_string();
                    }
                }
                else if(absolut<=20 && absolut>=5){
                    koda+="01";//5 bitov
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<4> vred3(absolut-5);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<4> vred3(absolut-5);
                        koda+=vred3.to_string();
                    }
                }
                else if(absolut<=276 && absolut>=21){
                    koda+="10";//9 bitov
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<8> vred3(absolut-21);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<8> vred3(absolut-21);
                        koda+=vred3.to_string();
                    }
                }
                else if(absolut<=4095 && absolut>=277){
                    koda+="11";//13 bitov
                    if(vet_zdej[i]<0){
                        koda+="0";
                        bitset<12> vred3(absolut-277);
                        koda+=vred3.to_string();
                    }
                    else if(vet_zdej[i]>0){
                        koda+="1";
                        bitset<12> vred3(absolut-277);
                        koda+=vred3.to_string();
                    }
                }
                i++;
            }//KONEC ABSOLUTNEGA
        }//ce ni prvi
    }//konec, cez vse elemente
}
void MainWindow::razlika_dveh_plasti(){
    for(int i=0;i<vet.size();i++){
            vet_zdej[i]=vet[i]-vet_prej[i];
    }
}
void MainWindow::string_v_vector(){
    telo.clear();
    unsigned char byte = 8;
    int lokacija=8;
    //iz stringa v binarni zapis-bit shifting
    for(int i=0;i<koda.length();i++){
                if(lokacija==0){//resitiram in zapisem
                    telo.push_back(byte);
                    byte=0;
                    lokacija=8;
                }
                //shiftanje
                byte<<=1; //za 1 premaknem
                if(koda[i]=='1'){
                    byte|=1;
                }
                else if(koda[i]=='0'){
                    byte|=0;
                }
                //byte|=(unsigned char)koda[i]; //OR operator -zdruzim
                lokacija=lokacija-1;
    }
    byte<<=lokacija;//pushnem za toliko kokr mi je se ostalo - da dobim celi byte
    telo.push_back(byte);
}

void MainWindow::on_actionCM3_Dekodiraj_triggered()
{
    QString imedat = QFileDialog::getOpenFileName(this,tr("Izberi"),"",tr("CMP3 datoteka (*.cmp3)"));
    if(QString::compare(imedat,QString())!=0){//ce sem izbral pot
        QByteArray fileNameArr = QFile::encodeName(imedat);
        const char * fileNameStr = fileNameArr.constData();//v tekstu pot
        ifstream infile(fileNameStr, ios::in | ios::binary);//odprem datoteko-binarno
        char c; //bere znak
        vet.clear();
        koda="";
        while(infile.get(c))//berem do konca - pridobivam byte
        {
            for(int i=0;i<8;i++){//za vsak bit v bytu
                char bit= c & 0x80; //da dobim prvega 1000000
                c<<=1; //premaknem v levo za 1
                if(bit==0){//ce je 0
                    koda+="0";
                }
                else{//ce je 1
                    koda+="1";
                }
            }
        }
        int index=0;
        string loc="";
        string stev="";
        string ime="";
        int stevilo;
        cout<<"dekodiranje cm3\n";
        //glava
        //locljivost
        loc=koda.substr(index,16);//vzamem 16 - locljivost
        index+=16;//pomaknem naprej
        bitset<16> a(loc);//spravim v decimalno
        struct C c1 = { static_cast<signed short>(a.to_ulong()) };
        N=c1.b;
        cout<<N<<"\n";
        //stevilo
        stev=koda.substr(index,16);
        index+=16;//pomaknem naprej
        bitset<16> b(stev);//spravim v decimalno
        struct C c2 = { static_cast<signed short>(b.to_ulong()) };
        stevilo=c2.b;
        cout<<stevilo<<"\n";
        string imena[stevilo];
        //imena
        for(int i =0; i<stevilo;i++){
            ime=koda.substr(index,16);//vzamem 16 - locljivost
            index+=16;//pomaknem naprej
            bitset<16> r(ime);//spravim v decimalno
            int imest=r.to_ulong();
            ostringstream ss;
            ss << imest;
            imena[i]=ss.str();
            cout<<imena[i]<<"\n";
       }
       int stevka=0;
       int st_zapisani_datotek=0;
       string prva="";
       string druga="";
       string tretji="";
       vet.clear();
       while(index<=koda.length()){//pridem do konca
           if(st_zapisani_datotek==0){//PRVA DATOTEKA
               prva=koda.substr(index,2);//vzamem 2
             //  cout<<"prva_koda:"<<prva<<"\n";
               index+=2;//pomaknem naprej
               if(prva=="11"){//ce je zrak
                   vet.insert(vet.end(),(signed short)-2048);
                   stevka++;
                   if(stevka>=N*N){
                       imena[st_zapisani_datotek].append(".img");
                       obdelam_vektor(N,imena[st_zapisani_datotek]);
                       vet_prej.clear();
                       vet_prej=vet;
                       vet.clear();
                       st_zapisani_datotek++;
                       stevka=0;
                       if(st_zapisani_datotek>=stevilo){vet=vet_prej;break;}
                   }
               }
               else if(prva=="00"){//ce je razlika
                   druga=koda.substr(index,2);//vzamem naslednja 2
                   index+=2;//pomaknem naprej
                   if(druga=="00"){//2 bita
                       tretji=koda.substr(index,2);//vzamem naslednja 2
                       index+=2;//pomaknem naprej
                       if(tretji=="00"){vet.insert(vet.end(),vet.back()-2);}
                       else if(tretji=="01"){vet.insert(vet.end(),vet.back()-1);}
                       else if(tretji=="10"){vet.insert(vet.end(),vet.back()+1);}
                       else if(tretji=="11"){vet.insert(vet.end(),vet.back()+2);}
                   }
                   else if(druga=="01"){//3 biti
                       tretji=koda.substr(index,3);//vzamem naslednjih 3
                       index+=3;//pomaknem naprej
                       if(tretji=="000"){vet.insert(vet.end(),vet.back()-6);}
                       else if(tretji=="001"){vet.insert(vet.end(),vet.back()-5);}
                       else if(tretji=="010"){vet.insert(vet.end(),vet.back()-4);}
                       else if(tretji=="011"){vet.insert(vet.end(),vet.back()-3);}
                       else if(tretji=="100"){vet.insert(vet.end(),vet.back()+3);}
                       else if(tretji=="101"){vet.insert(vet.end(),vet.back()+4);}
                       else if(tretji=="110"){vet.insert(vet.end(),vet.back()+5);}
                       else if(tretji=="111"){vet.insert(vet.end(),vet.back()+6);}
                   }
                   else if(druga=="10"){//4 biti
                       tretji=koda.substr(index,4);//vzamem naslednjih 4
                       index+=4;//pomaknem naprej
                       if(tretji=="0000"){vet.insert(vet.end(),vet.back()-14);}
                       else if(tretji=="0001"){vet.insert(vet.end(),vet.back()-13);}
                       else if(tretji=="0010"){vet.insert(vet.end(),vet.back()-12);}
                       else if(tretji=="0011"){vet.insert(vet.end(),vet.back()-11);}
                       else if(tretji=="0100"){vet.insert(vet.end(),vet.back()-10);}
                       else if(tretji=="0101"){vet.insert(vet.end(),vet.back()-9);}
                       else if(tretji=="0110"){vet.insert(vet.end(),vet.back()-8);}
                       else if(tretji=="0111"){vet.insert(vet.end(),vet.back()-7);}
                       else if(tretji=="1000"){vet.insert(vet.end(),vet.back()+7);}
                       else if(tretji=="1001"){vet.insert(vet.end(),vet.back()+8);}
                       else if(tretji=="1010"){vet.insert(vet.end(),vet.back()+9);}
                       else if(tretji=="1011"){vet.insert(vet.end(),vet.back()+10);}
                       else if(tretji=="1100"){vet.insert(vet.end(),vet.back()+11);}
                       else if(tretji=="1101"){vet.insert(vet.end(),vet.back()+12);}
                       else if(tretji=="1110"){vet.insert(vet.end(),vet.back()+13);}
                       else if(tretji=="1111"){vet.insert(vet.end(),vet.back()+14);}
                   }
                   else if(druga=="11"){//5 bitov
                       tretji=koda.substr(index,5);//vzamem naslednjih 5
                       index+=5;//pomaknem naprej
                       if(tretji=="00000"){vet.insert(vet.end(),vet.back()-30);}
                       else if(tretji=="00001"){vet.insert(vet.end(),vet.back()-29);}
                       else if(tretji=="00010"){vet.insert(vet.end(),vet.back()-28);}
                       else if(tretji=="00011"){vet.insert(vet.end(),vet.back()-27);}
                       else if(tretji=="00100"){vet.insert(vet.end(),vet.back()-26);}
                       else if(tretji=="00101"){vet.insert(vet.end(),vet.back()-25);}
                       else if(tretji=="00110"){vet.insert(vet.end(),vet.back()-24);}
                       else if(tretji=="00111"){vet.insert(vet.end(),vet.back()-23);}
                       else if(tretji=="01000"){vet.insert(vet.end(),vet.back()-22);}
                       else if(tretji=="01001"){vet.insert(vet.end(),vet.back()-21);}
                       else if(tretji=="01010"){vet.insert(vet.end(),vet.back()-20);}
                       else if(tretji=="01011"){vet.insert(vet.end(),vet.back()-19);}
                       else if(tretji=="01100"){vet.insert(vet.end(),vet.back()-18);}
                       else if(tretji=="01101"){vet.insert(vet.end(),vet.back()-17);}
                       else if(tretji=="01110"){vet.insert(vet.end(),vet.back()-16);}
                       else if(tretji=="01111"){vet.insert(vet.end(),vet.back()-15);}
                       else if(tretji=="10000"){vet.insert(vet.end(),vet.back()+15);}
                       else if(tretji=="10001"){vet.insert(vet.end(),vet.back()+16);}
                       else if(tretji=="10010"){vet.insert(vet.end(),vet.back()+17);}
                       else if(tretji=="10011"){vet.insert(vet.end(),vet.back()+18);}
                       else if(tretji=="10100"){vet.insert(vet.end(),vet.back()+19);}
                       else if(tretji=="10101"){vet.insert(vet.end(),vet.back()+20);}
                       else if(tretji=="10110"){vet.insert(vet.end(),vet.back()+21);}
                       else if(tretji=="10111"){vet.insert(vet.end(),vet.back()+22);}
                       else if(tretji=="11000"){vet.insert(vet.end(),vet.back()+23);}
                       else if(tretji=="11001"){vet.insert(vet.end(),vet.back()+24);}
                       else if(tretji=="11010"){vet.insert(vet.end(),vet.back()+25);}
                       else if(tretji=="11011"){vet.insert(vet.end(),vet.back()+26);}
                       else if(tretji=="11100"){vet.insert(vet.end(),vet.back()+27);}
                       else if(tretji=="11101"){vet.insert(vet.end(),vet.back()+28);}
                       else if(tretji=="11110"){vet.insert(vet.end(),vet.back()+29);}
                       else if(tretji=="11111"){vet.insert(vet.end(),vet.back()+30);}
                   }//konec 5 bitov
                   stevka++;
                   if(stevka>=N*N){
                       imena[st_zapisani_datotek].append(".img");
                       obdelam_vektor(N,imena[st_zapisani_datotek]);
                       vet_prej.clear();
                       vet_prej=vet;
                       vet.clear();
                       st_zapisani_datotek++;
                       stevka=0;
                       if(st_zapisani_datotek>=stevilo){break;}
                   }
               }//konec razlike
               else if(prva=="01"){//ponovitve
                   druga=koda.substr(index,6);//vzamem naslednja 2
                   index+=6;//pomaknem naprej
                   bitset<6> b(druga);//spravim v decimalno
                   int stevec=(int)b.to_ulong();
                   for(int i=0;i<stevec;i++){
                      vet.insert(vet.end(),vet.back());//dodam zadnjega na konec
                      stevka++;
                      if(stevka>=N*N){
                          imena[st_zapisani_datotek].append(".img");
                          obdelam_vektor(N,imena[st_zapisani_datotek]);
                          vet_prej.clear();
                          vet_prej=vet;
                          vet.clear();
                          st_zapisani_datotek++;
                          stevka=0;
                          if(st_zapisani_datotek>=stevilo){break;}
                      }
                   }
               }
               else if(prva=="10"){//absolutno
                   druga=koda.substr(index,12);//vzamem naslednja 2
                   index+=12;//pomaknem naprej
                   bitset<12> b(druga);//spravim v decimalno
                   struct C c = { static_cast<signed short>(b.to_ulong()) };
                   //cout<<(signed short)b.to_ulong()<<"\n";
                   vet.insert(vet.end(),c.b);//vstavim stevilko
                   stevka++;
                   if(stevka>=N*N){
                       imena[st_zapisani_datotek].append(".img");
                       obdelam_vektor(N,imena[st_zapisani_datotek]);
                       vet_prej.clear();
                       vet_prej=vet;
                       vet.clear();
                       st_zapisani_datotek++;
                       stevka=0;
                       if(st_zapisani_datotek>=stevilo){break;}
                   }
               }
           }//Konec ce je prva datoteka
           else{//DRUGE datoteke
               prva=koda.substr(index,2);//vzamem 2
               index+=2;//pomaknem naprej
               if(prva=="11"){//ce je zrak
                   vet.insert(vet.end(),vet_prej[stevka]);//vstavim isteka kot predhodno
                   stevka++;
                   if(stevka>=N*N){
                       imena[st_zapisani_datotek].append(".img");
                       obdelam_vektor(N,imena[st_zapisani_datotek]);
                       vet_prej.clear();
                       vet_prej=vet;
                       vet.clear();
                       st_zapisani_datotek++;
                       stevka=0;
                       if(st_zapisani_datotek>=stevilo){vet=vet_prej;break;}
                   }
               }
               else if(prva=="00"){//ce je razlika
                   druga=koda.substr(index,2);//vzamem naslednja 2
                   index+=2;//pomaknem naprej
                   if(druga=="00"){//2 bita
                       tretji=koda.substr(index,2);//vzamem naslednja 2
                       index+=2;//pomaknem naprej
                       if(tretji=="00"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-2);}
                       else if(tretji=="01"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-1);}
                       else if(tretji=="10"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+1);}
                       else if(tretji=="11"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+2);}
                   }
                   else if(druga=="01"){//3 biti
                       tretji=koda.substr(index,3);//vzamem naslednjih 3
                       index+=3;//pomaknem naprej
                       if(tretji=="000"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-6);}
                       else if(tretji=="001"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-5);}
                       else if(tretji=="010"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-4);}
                       else if(tretji=="011"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-3);}
                       else if(tretji=="100"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+3);}
                       else if(tretji=="101"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+4);}
                       else if(tretji=="110"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+5);}
                       else if(tretji=="111"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+6);}
                   }
                   else if(druga=="10"){//4 biti
                       tretji=koda.substr(index,4);//vzamem naslednjih 4
                       index+=4;//pomaknem naprej
                       if(tretji=="0000"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-14);}
                       else if(tretji=="0001"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-13);}
                       else if(tretji=="0010"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-12);}
                       else if(tretji=="0011"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-11);}
                       else if(tretji=="0100"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-10);}
                       else if(tretji=="0101"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-9);}
                       else if(tretji=="0110"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-8);}
                       else if(tretji=="0111"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-7);}
                       else if(tretji=="1000"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+7);}
                       else if(tretji=="1001"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+8);}
                       else if(tretji=="1010"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+9);}
                       else if(tretji=="1011"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+10);}
                       else if(tretji=="1100"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+11);}
                       else if(tretji=="1101"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+12);}
                       else if(tretji=="1110"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+13);}
                       else if(tretji=="1111"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+14);}
                   }
                   else if(druga=="11"){//5 bitov
                       tretji=koda.substr(index,5);//vzamem naslednjih 5
                       index+=5;//pomaknem naprej
                       if(tretji=="00000"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-30);}
                       else if(tretji=="00001"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-29);}
                       else if(tretji=="00010"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-28);}
                       else if(tretji=="00011"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-27);}
                       else if(tretji=="00100"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-26);}
                       else if(tretji=="00101"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-25);}
                       else if(tretji=="00110"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-24);}
                       else if(tretji=="00111"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-23);}
                       else if(tretji=="01000"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-22);}
                       else if(tretji=="01001"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-21);}
                       else if(tretji=="01010"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-20);}
                       else if(tretji=="01011"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-19);}
                       else if(tretji=="01100"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-18);}
                       else if(tretji=="01101"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-17);}
                       else if(tretji=="01110"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-16);}
                       else if(tretji=="01111"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])-15);}
                       else if(tretji=="10000"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+15);}
                       else if(tretji=="10001"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+16);}
                       else if(tretji=="10010"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+17);}
                       else if(tretji=="10011"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+18);}
                       else if(tretji=="10100"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+19);}
                       else if(tretji=="10101"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+20);}
                       else if(tretji=="10110"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+21);}
                       else if(tretji=="10111"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+22);}
                       else if(tretji=="11000"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+23);}
                       else if(tretji=="11001"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+24);}
                       else if(tretji=="11010"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+25);}
                       else if(tretji=="11011"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+26);}
                       else if(tretji=="11100"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+27);}
                       else if(tretji=="11101"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+28);}
                       else if(tretji=="11110"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+29);}
                       else if(tretji=="11111"){vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1])+30);}
                   }//konec 5 bitov
                   stevka++;
                   if(stevka>=N*N){
                       imena[st_zapisani_datotek].append(".img");
                       obdelam_vektor(N,imena[st_zapisani_datotek]);
                       vet_prej.clear();
                       vet_prej=vet;
                       vet.clear();
                       st_zapisani_datotek++;
                       stevka=0;
                       if(st_zapisani_datotek>=stevilo){vet=vet_prej;break;}
                   }
               }//konec razlike
               else if(prva=="01"){//ponovitve
                   druga=koda.substr(index,6);//vzamem naslednjih 6
                   index+=6;//pomaknem naprej
                   bitset<6> b(druga);//spravim v decimalno
                   int stevec=(int)b.to_ulong();
                   for(int i=0;i<stevec;i++){
                      vet.insert(vet.end(),vet_prej[stevka]+(vet.back()-vet_prej[stevka-1]));//dodam zadnjega na konec
                      stevka++;
                      if(stevka>=N*N){
                          imena[st_zapisani_datotek].append(".img");
                          obdelam_vektor(N,imena[st_zapisani_datotek]);
                          vet_prej.clear();
                          vet_prej=vet;
                          vet.clear();
                          st_zapisani_datotek++;
                          stevka=0;
                          if(st_zapisani_datotek>=stevilo){vet=vet_prej;break;}
                      }
                   }
               }
               else if(prva=="10"){//absolutno
                   druga=koda.substr(index,2);//vzamem naslednja 2
                   index+=2;//pomaknem naprej
                   if(druga=="00"){//kodirano s 3 biti
                       string predz=koda.substr(index,1);
                       index+=1;
                       tretji=koda.substr(index,2);
                       index+=2;
                       bitset<2> b(tretji);//spravim v decimalno
                       if(predz=="0"){
                        vet.insert(vet.end(),vet_prej[stevka]-(b.to_ulong()+1));//vstavim stevilko
                       }
                       else if(predz=="1"){
                         vet.insert(vet.end(),vet_prej[stevka]+(b.to_ulong()+1));//vstavim stevilko
                       }
                   }
                   else if(druga=="01"){//kodirano s 5 biti
                       string predz=koda.substr(index,1);
                       index+=1;
                       tretji=koda.substr(index,4);//vzamem naslednjih 4
                       index+=4;
                       bitset<4> b(tretji);//spravim v decimalno
                       if(predz=="0"){
                        vet.insert(vet.end(),vet_prej[stevka]-(b.to_ulong()+5));//vstavim stevilko
                       }
                       else if(predz=="1"){
                         vet.insert(vet.end(),vet_prej[stevka]+(b.to_ulong()+5));//vstavim stevilko
                       }
                   }
                   else if(druga=="10"){//kodirano s 9 biti
                       string predz=koda.substr(index,1);
                       index+=1;
                       tretji=koda.substr(index,8);//vzamem naslednjih 8
                       index+=8;
                       bitset<8> b(tretji);//spravim v decimalno
                       if(predz=="0"){
                        vet.insert(vet.end(),vet_prej[stevka]-(b.to_ulong()+21));//vstavim stevilko
                       }
                       else if(predz=="1"){
                         vet.insert(vet.end(),vet_prej[stevka]+(b.to_ulong()+21));//vstavim stevilko
                       }
                   }
                   else if(druga=="11"){//kodirano s 13 biti

                       string predz=koda.substr(index,1);
                       index+=1;
                       tretji=koda.substr(index,12);//vzamem naslednjih 12
                       index+=12;
                       bitset<12> b(tretji);//spravim v decimalno
                       if(predz=="0"){
                        vet.insert(vet.end(),vet_prej[stevka]-(b.to_ulong()+277));//vstavim stevilko
                       }
                       else if(predz=="1"){
                         vet.insert(vet.end(),vet_prej[stevka]+(b.to_ulong()+277));//vstavim stevilko
                       }
                   }

                   stevka++;

                   if(stevka>=N*N){
                       imena[st_zapisani_datotek].append(".img");
                       obdelam_vektor(N,imena[st_zapisani_datotek]);
                       vet_prej.clear();
                       vet_prej=vet;
                       vet.clear();
                       st_zapisani_datotek++;
                       stevka=0;
                       if(st_zapisani_datotek>=stevilo){vet=vet_prej;break;}
                   }
               }
           }//konec obdelave
          if(index>=koda.length()){
            cout<<"gotov sm";
          }
       }//konec while-a
     }//konec if.a
}
void MainWindow::obdelam_vektor(int N,string ime){
    N=ui->lineEdit->text().toInt();
    cout<<"obdelam vektor\n";
    QImage img(N, N, QImage::Format_RGB888);//ustvarim sliko
    img.fill(QColor(Qt::white).rgb());//z belo podlago
    int k=0;
    for(int i=0;i<N;i++){
        for(int j=0;j<N;j++){
           // cout<<"vet:"<<vet[k]<<"\n";
            //hitri prikaz brez palet v rdeci barvi
            int pix =(int)((((float)vet[k]+2048.f)/4095.f)*255.f);
            img.setPixel(i,j,qRgb(pix, 0, 0));
            k+=1;
        }
    }
   //prikaz
   ui->label->setPixmap(QPixmap::fromImage(img));
   ui->label->show();
   qApp->processEvents();//sprocesira
   ofstream outfile (ime.c_str() ,std::ios_base::binary);
   outfile.write((char*)(&vet[0]), N*N * sizeof(vet[0])) ;
   outfile.close();

}
