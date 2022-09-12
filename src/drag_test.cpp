#include <drag_rtmp.h>

DragRTMP::Ptr rtmp;

int main(int argc,char* argv[])
{
    string drag_addr = "rtmp://1.116.137.21:7788/zedm";
    // string drag_addr = "rtmp://1.116.137.21:7789/realsense_distort";

    rtmp = make_shared<DragRTMP>(30);

    if(!rtmp->initContext(drag_addr))
    {
        cout << "Failed to initialize RTMP context!" << endl;
        return -1;
    }

    while(true)
    {
        rtmp->startDrag();
        imshow("Drag video",rtmp->getImage());
        waitKey(5);
    }

    return 0;
}