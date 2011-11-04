#include "testApp.h"

#include "json.h"
#include <iostream>

//--------------------------------------------------------------
void
testApp::setup()
{
	ofSetFrameRate(30);

  previewSize = 20;
  previewFrame = -1;
  unsavedChanges = false;

  threshold   = 165;
  mouseFrame  = 0;
  paddingRows = 2;
  scroll      = 0;
  cols        = 10;
  numHues     = 10;
  load();
  rows        = ofGetHeight() / thumb.height;  

  nav.set(0, 0, ofGetWidth(), navThumb.height);

//startThread(true, false);

  saver.listCodecs();

	saver.setCodecType(21); // PhotoJPEG
//  saver.setCodecType(15); // H.264
	saver.setCodecQualityLevel(OF_QT_SAVER_CODEC_QUALITY_HIGH);

  ofAddListener(inputDevices.onMouseScroll, this, &testApp::mouseScroll);
  inputDevices.setup();
//  inputDevices.listDevices();
  
//  movie.setPlayer(ofPtr<ofBaseVideoPlayer>(new ofxFFmpegPlayer()));
//  movie.setPlayer(ofPtr<ofBaseVideoPlayer>(new ofxQTKitVideoPlayer()));
}

//--------------------------------------------------------------
void
testApp::exit()
{
  framebuffer.clear();
  previewbuffer.clear();
  
  framebufferRange = make_pair(0, 0);
  
  save();
  sourceFilename = "";
  originalSourceFilename = "";
  
  if (movie.isLoaded())
    movie.close();
  
  segments.clear();  
  currentSegment.clear();
}

//--------------------------------------------------------------
void
testApp::update()
{
  threadedFunction();
}

//--------------------------------------------------------------
void
testApp::draw()
{
  //ofGetWindowMode()
  //ofSetFullscreen(fullscreen);

  ofBackground(0, 0, 0);
  ofSetColor(255, 255, 255);

  ofPushMatrix();
  glEnable(GL_BLEND);       
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);  
  {
    ofTranslate(0, -scroll);

    int frames = movie.getTotalNumFrames();
    for (int i=0; i<framebuffer.size(); ++i)
    {
      int frame = framebufferRange.first + i;
      double pos = (double)frame / (double)frames;
      ofSetColor(255, 255, 255);

      ofRectangle rect = thumb;
      rect.x = ((int)(frame%cols))*thumb.width;
      rect.y = ((int)(frame/cols))*thumb.height;
      framebuffer[i].draw(rect);

      for (int i=0; i<segments.size(); ++i)
      {
        if (segments[i].startPos <= pos && pos <= segments[i].endPos)
        {
          ofSetColor(ofColor::fromHsb(ofMap(i%numHues, 0, numHues, 0., 200), 160, 127));
          ofRect(rect);
          break;
        }
      }
      
      if (keyframes.find(frame) != keyframes.end())
      {
        ofSetColor(ofColor::fromHsb(230, 160, 200));
        ofRect(rect);
      }

      if (frame == mouseFrame)
      {
        ofSetColor(127, 127, 127);
        ofRect(rect);
      }
    }
  }
  glDisable(GL_BLEND);
  ofSetColor(255, 255, 255, 255);
  ofPopMatrix();
  
  ofPushMatrix();
  glEnable(GL_BLEND);       
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_COLOR);  
  {
    ofTranslate(nav.x, nav.y);
    for (int i=0; i<previewbuffer.size(); ++i)
      previewbuffer[i].draw(nav.x+i*navThumb.width, nav.y, navThumb.width, navThumb.height);

    for (int i=0; i<segments.size(); ++i)
    {
      ofSetColor(ofColor::fromHsb(ofMap(i%numHues, 0, numHues, 0., 200), 160, 60));
      ofRect(ofMap(segments[i].startPos, 0., 1., 0, ofGetWidth()), 0,
             ofMap(segments[i].endPos-segments[i].startPos, 0., 1., 0, ofGetWidth()), navThumb.height);
    }

    if (previewFrame >= 0)
    {
      float x = ofMap(previewFrame, 0, movie.getTotalNumFrames(), nav.x, nav.x+nav.width);
      previewImg.draw(x - (navThumb.width/2), 0, navThumb.width, navThumb.height);
    }
  }
  glDisable(GL_BLEND);
  ofSetColor(255, 255, 255, 255);
  ofPopMatrix();
}

//--------------------------------------------------------------
void
testApp::keyPressed(int key)
{
  switch (key)
  {
    case OF_KEY_UP:
      threshold *= 1.1;
      keyframes.clear();
      std::cout << "threshold: " << threshold << std::endl;
      break;
    case OF_KEY_DOWN:
      threshold *= 0.9;
      keyframes.clear();
      std::cout << "threshold: " << threshold << std::endl;
      break;

    case 's':
      save();
      break;

    case 'S':
      unsavedChanges = true;
      save();
      break;
      
    case 'e':
      exportVideos();
      break;

    case 'o':
      load();
      break;
      
    case 'O':
      chooseOriginalSource();
      break;

    case 'f':
      exportFFmpegScript();
      break;
  }
}

//--------------------------------------------------------------
void
testApp::keyReleased(int key)
{}

//--------------------------------------------------------------
void
testApp::mouseMoved(int x, int y)
{
  updateMouseFrame(x, y, scroll);
}

//--------------------------------------------------------------
void
testApp::mouseDragged(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::mousePressed(int x, int y, int button)
{}

//--------------------------------------------------------------
void
testApp::mouseReleased(int x, int y, int button)
{
  if (movie.isLoaded())
  {
    unsigned int frames = movie.getTotalNumFrames();
    double pos = (double)mouseFrame / (double)frames;
    double incr = (double)1. / (double)frames;

    if (currentSegment.startPos < 0)
    {
      currentSegment.startPos = pos;
    }
    else if (currentSegment.startPos == currentSegment.endPos)
    {
      currentSegment.startPos = -1;
    }
    else if (currentSegment.startPos >= 0 && currentSegment.endPos < 0)
    {
      currentSegment.endPos = pos - incr;
      if (currentSegment.startPos > currentSegment.endPos)
        std::swap(currentSegment.startPos, currentSegment.endPos);

      if (currentSegment.name.empty())
        currentSegment.name = ofFilePath::getBaseName(sourceFilename)+"_"
          + ofToString((int)(currentSegment.startPos * frames))+"_"
          + ofToString((int)(currentSegment.endPos * frames));

      segments.push_back(currentSegment);
      unsavedChanges = true;
      currentSegment.clear();
    }
  }
}

//--------------------------------------------------------------
void
testApp::mouseScroll(ofxManyMouseEvent& event)
{
  updateMouseFrame(mouseX, mouseY, MAX(0, scroll + event.value));
}

//--------------------------------------------------------------
void
testApp::updateMouseFrame(int x, int y, int scroll)
{
  if (movie.isLoaded())
  {
    this->scroll = scroll;
    mouseFrame = ((int)((y+scroll) / thumb.height)) * cols;
    mouseFrame += (int)ofMap(x, 0, ofGetWidth(), 0, cols, true);
  }
}

//--------------------------------------------------------------
void
testApp::windowResized(int w, int h)
{}

//--------------------------------------------------------------
void
testApp::gotMessage(ofMessage msg)
{}

//--------------------------------------------------------------
void
testApp::dragEvent(ofDragInfo dragInfo)
{}

//--------------------------------------------------------------
void
testApp::load()
{
  ofFileDialogResult loadFile = ofSystemLoadDialog("Open video or segments file", false);
  if (loadFile.bSuccess)
    load(loadFile.getPath());
  else if (!movie.isLoaded()) // first run
    exitApp();
}

//--------------------------------------------------------------
void
testApp::load(const std::string& _filename)
{
  exit();

  std::string ext = ofFilePath::getFileExt(_filename);
  if (ext=="json")
  {
    ifstream jsonfile(_filename.c_str());
    Json::Value json;
    Json::Reader reader;
    bool parsed = reader.parse(jsonfile, json);
    jsonfile.close();
    if (!parsed)
    {
      std::cout
      << "Failed to parse segments file" << std::endl
      << reader.getFormatedErrorMessages();
      return;
    }

    const Json::Value segmentsJSON = json["segments"];
    segments.resize(segmentsJSON.size());
    for (int i=0; i<segmentsJSON.size(); ++i)
      segments[i].fromJSON(segmentsJSON[i]);

    sourceFilename = json.get("sourceFilename", "").asString();
    originalSourceFilename = json.get("originalSourceFilename", sourceFilename).asString();
  }
  else {
    sourceFilename = _filename;
    originalSourceFilename = _filename;
  }

  movie.loadMovie(sourceFilename);
//  player->loadMovie(sourceFilename, OFXQTVIDEOPLAYER_MODE_TEXTURE_ONLY);
//  player->loadMovie(sourceFilename, OFXQTVIDEOPLAYER_MODE_PIXELS_ONLY);
//  player->loadMovie(sourceFilename, OFXQTVIDEOPLAYER_MODE_PIXELS_AND_TEXTURE);

  if (movie.isLoaded())
  {
    frameImg.allocate(movie.getWidth(), movie.getHeight());
    diffImg.allocate(movie.getWidth(), movie.getHeight());
    previewImg.allocate(movie.getWidth(), movie.getHeight());
    
    float thumb_w, thumb_h;

    thumb_w = ofGetWidth() / (float)cols;
    thumb_h = thumb_w * movie.getHeight() / movie.getWidth();
    thumb.set(0, 0, thumb_w, thumb_h);
    
    thumb_w = ofGetWidth() / (float)previewSize;
    thumb_h = thumb_w * movie.getHeight() / movie.getWidth();
    navThumb.set(0, 0, thumb_w, thumb_h);
  }
}

//--------------------------------------------------------------
void
testApp::save()
{
  if (unsavedChanges)
  {
    std::string saveFilename = ofFilePath::getBaseName(sourceFilename)+"_segments.json";
    ofFileDialogResult saveFilePrompt = ofSystemSaveDialog(saveFilename, "Save segments file");
    saveFilename = saveFilePrompt.getPath();
    save(saveFilename);
  }
}

//--------------------------------------------------------------
void
testApp::save(const std::string& _filename)
{
  if (unsavedChanges)
  {
    ofstream jsonfile(_filename.c_str());
    Json::Value json;

    double incr = (double)1./(double)movie.getTotalNumFrames();
    json["sourceFilename"] = sourceFilename;
    json["originalSourceFilename"] = originalSourceFilename;
    for (int i=0; i<segments.size(); ++i)
    {
      //segments[i].endPos -= 2*incr;
      json["segments"].append(segments[i].toJSON());
    }

    jsonfile << json;
    jsonfile.close();
    unsavedChanges = false;
  }
}

//--------------------------------------------------------------
void
testApp::exportFFmpegScript()
{
  std::string ffmpegScriptFilename    = ofFilePath::getBaseName(sourceFilename)+"_segments_ffmpeg.sh";
  std::string originalSourceFilename  = sourceFilename;
  ofFileDialogResult saveFilePrompt   = ofSystemSaveDialog(ffmpegScriptFilename, "Save ffmpeg script");
  ffmpegScriptFilename = saveFilePrompt.getPath();

  exportFFmpegScript(ffmpegScriptFilename, originalSourceFilename);
}

//--------------------------------------------------------------
void
testApp::exportFFmpegScript(const std::string& ffmpegScriptFilename)
{
  exportFFmpegScript(ffmpegScriptFilename, sourceFilename);
}

//--------------------------------------------------------------
void
testApp::exportFFmpegScript(const std::string& ffmpegScriptFilename,
                            const std::string& originalSourceFilename)
{
  int frames = movie.getTotalNumFrames();
  double duration = movie.getDuration();

  ofstream ffmpegScriptFile(ffmpegScriptFilename.c_str());
  ffmpegScriptFile << "#!/bin/sh" << std::endl;

  int codec = 2;

  for (int i=0; i<segments.size(); ++i)
  {
    ffmpegScriptFile
    << "ffmpeg -i \"" + originalSourceFilename + "\"";
    
    switch (codec)
    {
      case 0: // Raw MPEG-2 dump
        ffmpegScriptFile << " -f mpeg2video -vcodec copy";
        break;
      case 1: // Filename-based codec selection, every frame a keyframe
        ffmpegScriptFile << " -sameq -g 1 -keyint_min 1";
        break;
      default:
      case 2: // libx264 psychovisually optimized codec, every frame a keyframe
        ffmpegScriptFile << " -vcodec libx264 -x264opts keyint=1:crf=12";
        break;
    }
    ffmpegScriptFile
    << " -an " // omit audio
    << " -ss " << ofMap(segments[i].startPos, 0., 1., 0., duration)
    << " -t " << ofMap(segments[i].endPos - segments[i].startPos, 0., 1., 0., duration)
    << " \"" << segments[i].name << ".mp4\""
    << std::endl;
  }
  ffmpegScriptFile.close();
}

//--------------------------------------------------------------
void
testApp::chooseOriginalSource()
{
  ofFileDialogResult originalSourcePrompt = ofSystemLoadDialog(sourceFilename, "Choose original source video");
  originalSourceFilename = originalSourcePrompt.getPath();
}

//--------------------------------------------------------------
void
testApp::exportVideos()
{
  if (movie.isLoaded())
  {
    std::string segmentFilename;
    for (int i=0; i<segments.size(); ++i)
    {
      segmentFilename = segments[i].name+".mov";
      if (!ofFile::doesFileExist(segmentFilename))
      {
        saver.setup(movie.getWidth(), movie.getHeight(), segmentFilename);
        int frames = movie.getTotalNumFrames();
        int startFrame = ceil(segments[i].startPos * (double)frames) + 1;
        int endFrame = floor(segments[i].endPos * (double)frames) - 1;
        for (int i=startFrame; i<endFrame; ++i)
        {
          movie.setFrame(i);
          movie.update();
          saver.addFrame(movie.getPixels(), 1./30.);
        }
        saver.finishMovie();
      }
    }
  }
}

//--------------------------------------------------------------
void
testApp::threadedFunction()
{
  if (movie.isLoaded())
  {
    if (previewbuffer.size() < previewSize)
    {
      previewbuffer.resize(previewSize);
      for (int i=0; i<previewbuffer.size(); ++i)
        previewbuffer[i].allocate(movie.getWidth(), movie.getHeight(), OF_IMAGE_COLOR);

      for (int i=0; i<previewbuffer.size(); ++i)
      {
        movie.setFrame(ofMap(i, 0, previewbuffer.size(), 0, movie.getTotalNumFrames()));
        movie.update();
        previewbuffer[i].setFromPixels(movie.getPixelsRef());
      }
    }

    if (nav.inside(mouseX, mouseY))
    {
      usingNav = true;
      previewFrame = ofMap(mouseX, nav.x, nav.x+nav.width, 0, movie.getTotalNumFrames(), true);
      movie.setFrame(previewFrame);
      movie.update();
      previewImg.setFromPixels(movie.getPixelsRef());
    }
    else {
      if (usingNav)
      {
        usingNav = false;
        updateMouseFrame(mouseX, mouseY, (previewFrame/(int)cols)*thumb.height);
        framebuffer.clear();
        framebufferRange = make_pair(previewFrame, previewFrame);
      }

      int frameRowStart = ofClamp((scroll/thumb.height)*cols, 0, movie.getTotalNumFrames());
      int newStartFrame = ofClamp(frameRowStart - ((paddingRows)*cols), 0, movie.getTotalNumFrames());
      int newEndFrame   = ofClamp(frameRowStart + ((rows+paddingRows)*cols), 0, movie.getTotalNumFrames());
      /* std::cout << "previewFrame:" << previewFrame << ", framebufferRange.first:" << framebufferRange.first << ", framebufferRange.second:" << framebufferRange.second << ", newStartFrame:" << newStartFrame << ", newEndFrame:" << newEndFrame << ", frameRowStart:" << frameRowStart <<  ", scroll:" << scroll << std::endl; */
      if (framebufferRange.first > newStartFrame)
      {
        movie.setFrame(framebufferRange.first);
        movie.update();
        for (int i=framebufferRange.first; i>newStartFrame; --i)
        {
          movie.previousFrame();
          movie.update();
          frameImg.setFromPixels(movie.getPixels(), movie.getWidth(), movie.getHeight());

          if (!framebuffer.empty()
              && abs(frameDiff(frameImg, framebuffer.front())) > threshold)
          {
            keyframes[i] = true;
          }

          framebuffer.push_front(frameImg);
        }
      }
      else if (newStartFrame > framebufferRange.first)
      {
        for (int i=framebufferRange.first; i<newStartFrame; ++i)
        {
          if (framebuffer.empty())
            break;
          framebuffer.pop_front();
        }
      }

      if (newEndFrame > framebufferRange.second)
      {
        movie.setFrame(framebufferRange.second);
        movie.update();
        for (int i=framebufferRange.second; i<newEndFrame; ++i)
        {
          movie.nextFrame();
          movie.update();
          frameImg.setFromPixels(movie.getPixels(), movie.getWidth(), movie.getHeight());

          if (!framebuffer.empty() &&
              abs(frameDiff(frameImg, framebuffer.back())) > threshold)
          {
            keyframes[i] = true;
  //            std::cout << "keyframe, " << i << std::endl;
          }

          framebuffer.push_back(frameImg);
        }
      }
      else if (framebufferRange.second > newEndFrame)
      {
        for (int i=newEndFrame; i<framebufferRange.second; ++i)
        {
          if (framebuffer.empty())
            break;
          framebuffer.pop_back(); 
        }
      }

      framebufferRange = make_pair(newStartFrame, newEndFrame);
    }
  }
}

//--------------------------------------------------------------
double
testApp::frameDiff(ofxCvColorImage frame, ofxCvColorImage prev)
{
  cv::Mat_<cv::Vec3b> frameMat(frame.getCvImage());
  cv::Mat_<cv::Vec3b> prevFrameMat(prev.getCvImage());
  cv::Mat_<cv::Vec3b> diffMat(diffImg.getCvImage());

/*
  cv::Scalar sum = cv::sum(frameMat);
  double green = sum[1] / (sum[0]+sum[1]+sum[2]);
  if (green > 0.8)
  {
    startFramePrev = startFrame;
    return;
  }
*/

  cv::absdiff(frameMat, prevFrameMat, diffMat);
  cv::Scalar sum = cv::sum(diffMat);
  double diff = (sum[0] + sum[1] + sum[2]) / (frame.width * frame.height);

  return diff;
}
