#pragma once

#include "ofMain.h"
#include "ofxQtVideoSaver.h"
#include "ofxManyMouse.h"
#include "ofThread.h"
#include "ofxOpenCv.h"

#include "Segment.h"

class testApp
: public ofBaseApp
, public ofThread
{
public:
  void setup();
  void update();
  void draw();
  void exit();

  void keyPressed  (int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y );
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  void threadedFunction();

protected:
  void load();
  void load(const std::string& _filename);

  void save();
  void save(const std::string& _filename);

  void exportFFmpegScript();
  void exportFFmpegScript(const std::string& ffmpegScriptFilename);
  void exportFFmpegScript(const std::string& ffmpegScriptFilename,
                          const std::string& originalSourceFilename);
  
  void exportVideos();
  void chooseOriginalSource();
  
  double frameDiff(ofxCvColorImage frame, ofxCvColorImage prev);

  void updateMouseFrame(int x, int y, int scroll);
  int mouseFrame;

  ofVideoPlayer movie;

  std::deque<ofxCvColorImage> framebuffer;
  std::pair<int, int> framebufferRange;
  std::map<int, bool> keyframes;

  std::vector<ofImage> previewbuffer;

  ofxQtVideoSaver	saver;
  Segment currentSegment;
  std::vector<Segment> segments;  

private:
  ofxCvColorImage frameImg;
  ofxCvColorImage previewImg;
  ofxCvColorImage diffImg;
  double threshold;
  int previewSize;
  int paddingRows;
  bool usingNav;
  int previewFrame;
  int numHues;

  ofxManyMouse inputDevices;
  void mouseScroll(ofxManyMouseEvent& event);
  int scroll;
  int rows, cols;

  std::string sourceFilename;
  std::string originalSourceFilename;

  ofRectangle thumb;
  ofRectangle nav;
  ofRectangle navThumb;
  
  ofTrueTypeFont font;
  bool unsavedChanges;
};
