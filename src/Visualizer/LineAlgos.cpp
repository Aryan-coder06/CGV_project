#include "../../include/Visualizer/LineAlgos.h"
#include <cmath>
#include <algorithm>
#include <sstream>
#include <iomanip>

// ---------------------------------------------------------------
// Formatting helpers (shared across all algorithms in this file)
// ---------------------------------------------------------------
static std::string f4(float v) {
    std::ostringstream o; o << std::fixed << std::setprecision(4) << v; return o.str();
}
static std::string f2(float v) {
    std::ostringstream o; o << std::fixed << std::setprecision(2) << v; return o.str();
}
static std::string si(int v) { return std::to_string(v); }
static std::string pct(float v) {     // e.g. 0.75 -> "75%"
    return si((int)std::round(v * 100.0f)) + "%";
}

// ================================================================
//  DDALine
// ================================================================
DDALine::DDALine()
    : x1(0),y1(0),x2(0),y2(0),currentX(0),currentY(0),
      xIncrement(0),yIncrement(0),totalSteps(0),currentStep(0) {}

void DDALine::calculateIncrements() {
    int dx=x2-x1, dy=y2-y1;
    totalSteps = std::max(std::abs(dx), std::abs(dy));
    if (totalSteps>0) { xIncrement=(float)dx/totalSteps; yIncrement=(float)dy/totalSteps; }
    else               { xIncrement=yIncrement=0; }
}

void DDALine::init(int sx,int sy,int ex,int ey) {
    x1=sx;y1=sy;x2=ex;y2=ey;
    currentX=(float)x1; currentY=(float)y1;
    currentStep=0; path.clear();
    calculateIncrements();
    path.push_back({(int)std::round(currentX),(int)std::round(currentY),1.0f});
    lastState=AlgoState(); lastState.totalSteps=totalSteps; lastState.pixelsPerStep=1;
}

void DDALine::step() {
    if(isFinished()) return;
    float px=currentX, py=currentY;
    currentX+=xIncrement; currentY+=yIncrement; currentStep++;
    int rx=(int)std::round(currentX), ry=(int)std::round(currentY);
    path.push_back({rx,ry,1.0f});
    lastState.hasCalculation=true; lastState.currentStep=currentStep;
    lastState.totalSteps=totalSteps; lastState.pixelsPerStep=1;
    lastState.calcLines.clear();
    lastState.calcLines.push_back("--- X update ---");
    lastState.calcLines.push_back("  x  =  x_prev + xInc");
    lastState.calcLines.push_back("  x  =  "+f4(px)+" + "+f4(xIncrement)+" = "+f4(currentX));
    lastState.calcLines.push_back("--- Y update ---");
    lastState.calcLines.push_back("  y  =  y_prev + yInc");
    lastState.calcLines.push_back("  y  =  "+f4(py)+" + "+f4(yIncrement)+" = "+f4(currentY));
    lastState.calcLines.push_back("--- Rounding to nearest pixel ---");
    lastState.calcLines.push_back("  round("+f4(currentX)+")  =  "+si(rx));
    lastState.calcLines.push_back("  round("+f4(currentY)+")  =  "+si(ry));
    lastState.currentPixelInfo=">> Pixel plotted: ("+si(rx)+", "+si(ry)+")";
}

void DDALine::stepK(int k) {
    for(int i=0;i<k&&!isFinished();++i){
        currentX+=xIncrement; currentY+=yIncrement; currentStep++;
        path.push_back({(int)std::round(currentX),(int)std::round(currentY),1.0f});
    }
    lastState.hasCalculation=false; lastState.currentStep=currentStep;
    lastState.totalSteps=totalSteps; lastState.calcLines.clear(); lastState.currentPixelInfo="";
}

void DDALine::runToCompletion() {
    while(!isFinished()){ currentX+=xIncrement; currentY+=yIncrement; currentStep++;
        path.push_back({(int)std::round(currentX),(int)std::round(currentY),1.0f}); }
    lastState.hasCalculation=false; lastState.currentStep=currentStep;
    lastState.totalSteps=totalSteps; lastState.calcLines.clear();
}

void DDALine::reset()                        { init(x1,y1,x2,y2); }
bool DDALine::isFinished() const             { return currentStep>=totalSteps; }
std::vector<Pixel> DDALine::getHighlightedPixels() const { return path; }
AlgoState DDALine::getCurrentState() const   { return lastState; }
std::string DDALine::getName() const         { return "DDA  (Digital Differential Analyzer)"; }

std::vector<std::string> DDALine::getInitInfo() const {
    int dx=x2-x1,dy=y2-y1;
    int s=std::max(std::abs(dx),std::abs(dy));
    float xi=s>0?(float)dx/s:0, yi=s>0?(float)dy/s:0;
    return {"dx = x2-x1  =  "+si(x2)+"-"+si(x1)+" = "+si(dx),
            "dy = y2-y1  =  "+si(y2)+"-"+si(y1)+" = "+si(dy),
            "steps = max(|"+si(dx)+"|,|"+si(dy)+"|) = "+si(s),
            "xInc = "+si(dx)+"/"+si(s)+" = "+f4(xi),
            "yInc = "+si(dy)+"/"+si(s)+" = "+f4(yi)};
}
std::vector<std::string> DDALine::getCurrentVars() const {
    return {"Float X  : "+f4(currentX),"Float Y  : "+f4(currentY),
            "xInc     : "+f4(xIncrement),"yInc     : "+f4(yIncrement),
            "Pixel    : ("+si((int)std::round(currentX))+", "+si((int)std::round(currentY))+")",
            "Plotted  : "+si((int)path.size())+" pixels"};
}
std::string DDALine::getTheory() const {
    return
        "DDA — Digital Differential Analyzer\n"
        "====================================\n\n"
        "WHAT IS IT?\n"
        "  The DDA algorithm is one of the earliest and simplest\n"
        "  line rasterization techniques in Computer Graphics.\n"
        "  It converts a mathematical line defined by two endpoints\n"
        "  into a series of discrete pixels on a grid.\n\n"
        "CORE IDEA:\n"
        "  Instead of computing y = mx + c for every pixel\n"
        "  (which requires a multiplication per step), DDA adds\n"
        "  a constant floating-point increment to both x and y\n"
        "  at every iteration — only an addition per step.\n\n"
        "ALGORITHM STEPS:\n"
        "  1. Compute:   dx = x2 - x1\n"
        "                dy = y2 - y1\n"
        "  2. Compute:   steps = max( |dx|, |dy| )\n"
        "  3. Compute:   xInc = dx / steps\n"
        "                yInc = dy / steps\n"
        "  4. Start:     x = x1,  y = y1\n"
        "  5. Repeat steps times:\n"
        "       x = x + xInc\n"
        "       y = y + yInc\n"
        "       Plot pixel at ( round(x), round(y) )\n\n"
        "WORKED EXAMPLE  (0,0) -> (5,3):\n"
        "  dx=5, dy=3, steps=5, xInc=1.0, yInc=0.6\n"
        "  (0,0) -> (1,1) -> (2,1) -> (3,2) -> (4,2) -> (5,3)\n\n"
        "ADVANTAGES:\n"
        "+   Simple to implement\n"
        "+   Only addition per step\n\n"
        "DISADVANTAGES:\n"
        "-   Floating-point: slower on early hardware\n"
        "-   Rounding error accumulates for long lines\n"
        "-   Bresenham fixes this with integers only\n\n"
        "TIME COMPLEXITY:  O( max(|dx|,|dy|) )\n"
        "SPACE COMPLEXITY: O( n )\n";
}

// ================================================================
//  BresenhamLine
// ================================================================
BresenhamLine::BresenhamLine()
    : x1(0),y1(0),x2(0),y2(0),x(0),y(0),dx(0),dy(0),
      sx(1),sy(1),p(0),steep(false),totalSteps(0),currentStep(0) {}

void BresenhamLine::init(int startX,int startY,int endX,int endY) {
    x1=startX;y1=startY;x2=endX;y2=endY;
    x=x1;y=y1;
    dx=std::abs(x2-x1); dy=std::abs(y2-y1);
    sx=(x2>=x1)?1:-1; sy=(y2>=y1)?1:-1;
    steep=(dy>dx);
    path.clear(); currentStep=0;
    path.push_back({x,y,1.0f});
    if(!steep){p=2*dy-dx;totalSteps=dx;}
    else      {p=2*dx-dy;totalSteps=dy;}
    lastState=AlgoState(); lastState.totalSteps=totalSteps; lastState.pixelsPerStep=1;
}

void BresenhamLine::step() {
    if(isFinished()) return;
    int op=p,ox_=x,oy_=y; bool ym=false,xm=false;
    if(!steep){ x+=sx; if(p<0){p+=2*dy;}else{y+=sy;ym=true;p+=2*dy-2*dx;} }
    else      { y+=sy; if(p<0){p+=2*dx;}else{x+=sx;xm=true;p+=2*dx-2*dy;} }
    currentStep++; path.push_back({x,y,1.0f});
    lastState.hasCalculation=true; lastState.currentStep=currentStep;
    lastState.totalSteps=totalSteps; lastState.pixelsPerStep=1;
    lastState.calcLines.clear();
    if(!steep){
        lastState.calcLines.push_back("--- Mode: Gentle slope (|dx|>=|dy|) ---");
        lastState.calcLines.push_back("  Primary axis: X");
        lastState.calcLines.push_back("  x = "+si(ox_)+" + "+si(sx)+" = "+si(x));
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("--- Decision Parameter ---");
        lastState.calcLines.push_back("  p (before) = "+si(op));
        if(!ym){
            lastState.calcLines.push_back("  p < 0 ?  YES  ("+si(op)+" < 0)");
            lastState.calcLines.push_back("  Decision: Y stays at "+si(oy_));
            lastState.calcLines.push_back("  p = p + 2*dy = "+si(op)+" + "+si(2*dy)+" = "+si(p));
        } else {
            lastState.calcLines.push_back("  p >= 0 ?  YES  ("+si(op)+" >= 0)");
            lastState.calcLines.push_back("  Decision: Y increments");
            lastState.calcLines.push_back("  y = "+si(oy_)+" + "+si(sy)+" = "+si(y));
            lastState.calcLines.push_back("  p = p + 2*dy - 2*dx = "+si(op)+" + "+si(2*dy)+" - "+si(2*dx)+" = "+si(p));
        }
    } else {
        lastState.calcLines.push_back("--- Mode: Steep slope (|dy|>|dx|) ---");
        lastState.calcLines.push_back("  Primary axis: Y");
        lastState.calcLines.push_back("  y = "+si(oy_)+" + "+si(sy)+" = "+si(y));
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("--- Decision Parameter ---");
        lastState.calcLines.push_back("  p (before) = "+si(op));
        if(!xm){
            lastState.calcLines.push_back("  p < 0 ?  YES  ("+si(op)+" < 0)");
            lastState.calcLines.push_back("  Decision: X stays at "+si(ox_));
            lastState.calcLines.push_back("  p = p + 2*dx = "+si(op)+" + "+si(2*dx)+" = "+si(p));
        } else {
            lastState.calcLines.push_back("  p >= 0 ?  YES  ("+si(op)+" >= 0)");
            lastState.calcLines.push_back("  Decision: X increments");
            lastState.calcLines.push_back("  x = "+si(ox_)+" + "+si(sx)+" = "+si(x));
            lastState.calcLines.push_back("  p = p + 2*dx - 2*dy = "+si(op)+" + "+si(2*dx)+" - "+si(2*dy)+" = "+si(p));
        }
    }
    lastState.currentPixelInfo=">> Pixel plotted: ("+si(x)+", "+si(y)+")";
}

void BresenhamLine::stepK(int k) {
    for(int i=0;i<k&&!isFinished();++i){
        if(!steep){x+=sx;if(p<0)p+=2*dy;else{y+=sy;p+=2*dy-2*dx;}}
        else      {y+=sy;if(p<0)p+=2*dx;else{x+=sx;p+=2*dx-2*dy;}}
        currentStep++; path.push_back({x,y,1.0f});
    }
    lastState.hasCalculation=false; lastState.currentStep=currentStep;
    lastState.totalSteps=totalSteps; lastState.calcLines.clear(); lastState.currentPixelInfo="";
}
void BresenhamLine::runToCompletion() {
    while(!isFinished()){
        if(!steep){x+=sx;if(p<0)p+=2*dy;else{y+=sy;p+=2*dy-2*dx;}}
        else      {y+=sy;if(p<0)p+=2*dx;else{x+=sx;p+=2*dx-2*dy;}}
        currentStep++; path.push_back({x,y,1.0f});
    }
    lastState.hasCalculation=false; lastState.currentStep=currentStep; lastState.totalSteps=totalSteps; lastState.calcLines.clear();
}
void BresenhamLine::reset()                        { init(x1,y1,x2,y2); }
bool BresenhamLine::isFinished() const             { return currentStep>=totalSteps; }
std::vector<Pixel> BresenhamLine::getHighlightedPixels() const { return path; }
AlgoState BresenhamLine::getCurrentState() const   { return lastState; }
std::string BresenhamLine::getName() const         { return "Bresenham's Line Algorithm"; }

std::vector<std::string> BresenhamLine::getInitInfo() const {
    std::string mode=steep?"Steep (|dy|>|dx|) — primary: Y":"Gentle (|dx|>=|dy|) — primary: X";
    int p0=steep?2*dx-dy:2*dy-dx;
    std::string pf=steep?"p0 = 2*dx-dy = 2*"+si(dx)+"-"+si(dy)+"="+si(p0)
                        :"p0 = 2*dy-dx = 2*"+si(dy)+"-"+si(dx)+"="+si(p0);
    return {"dx=|"+si(x2)+"-"+si(x1)+"|="+si(dx)+"  dy=|"+si(y2)+"-"+si(y1)+"|="+si(dy),
            "sx="+si(sx)+" (X direction)  sy="+si(sy)+" (Y direction)",
            "Slope mode: "+mode, pf,
            "Total steps = "+si(totalSteps)};
}
std::vector<std::string> BresenhamLine::getCurrentVars() const {
    return {"Current X  : "+si(x),"Current Y  : "+si(y),
            "Decision p : "+si(p),"dx         : "+si(dx),
            "dy         : "+si(dy),"Plotted    : "+si((int)path.size())+" pixels"};
}
std::string BresenhamLine::getTheory() const {
    return
        "Bresenham's Line Algorithm\n"
        "==========================\n\n"
        "WHAT IS IT?\n"
        "  Developed by Jack Elton Bresenham at IBM in 1962.\n"
        "  Draws a raster line using ONLY integer arithmetic\n"
        "  (add/subtract only) — no floating-point.\n\n"
        "CORE IDEA:\n"
        "  Maintain an integer 'decision parameter' p.\n"
        "  The sign of p tells whether the secondary axis\n"
        "  should increment or stay.\n\n"
        "GENTLE slope |dx| >= |dy|:  Primary step = X\n"
        "  p0 = 2*dy - dx\n"
        "  Each step: x += sx  (always)\n"
        "    if p < 0:  p += 2*dy          (Y stays)\n"
        "    else:      y += sy             (Y steps)\n"
        "               p += 2*dy - 2*dx\n\n"
        "STEEP slope |dy| > |dx|:  Primary step = Y\n"
        "  p0 = 2*dx - dy\n"
        "  Each step: y += sy  (always)\n"
        "    if p < 0:  p += 2*dx          (X stays)\n"
        "    else:      x += sx             (X steps)\n"
        "               p += 2*dx - 2*dy\n\n"
        "ADVANTAGES vs DDA:\n"
        "+   Pure integer arithmetic — no rounding errors\n"
        "+   Faster on hardware without FPU\n\n"
        "DISADVANTAGES:\n"
        "-   Does not support anti-aliasing natively\n"
        "-   Xiaolin Wu's algorithm solves that\n\n"
        "TIME COMPLEXITY:  O( max(|dx|,|dy|) )\n"
        "SPACE COMPLEXITY: O( n )\n";
}

// ================================================================
//  XiaolinWuLine
// ================================================================
XiaolinWuLine::XiaolinWuLine()
    : ox0(0),oy0(0),ox1(0),oy1(0),
      fpx0(0),fpy0(0),fpx1(0),fpy1(0),
      steep(false),gradient(0),
      xpxl1(0),xpxl2(0),yend1(0),xgap1(0),yend2(0),xgap2(0),
      intery(0),currentX(0),currentStep(0),totalSteps(0) {}

// ---------------------------------------------------------------
//  Plot one pixel — un-swaps steep transformation
// ---------------------------------------------------------------
void XiaolinWuLine::plotPixel(int x, int y, float intensity) {
    intensity = std::max(0.0f, std::min(1.0f, intensity));
    if (steep) path.push_back({y, x, intensity});
    else       path.push_back({x, y, intensity});
}

// ---------------------------------------------------------------
//  doStep() — raw single-step without recording calculations
// ---------------------------------------------------------------
void XiaolinWuLine::doStep() {
    if (isFinished()) return;
    if (currentX == xpxl1) {
        // First endpoint
        int iy = wu_ipart(yend1);
        plotPixel(xpxl1, iy,   wu_rfrac(yend1) * xgap1);
        plotPixel(xpxl1, iy+1, wu_frac(yend1)  * xgap1);
    } else if (currentX == xpxl2) {
        // Last endpoint
        int iy = wu_ipart(yend2);
        plotPixel(xpxl2, iy,   wu_rfrac(yend2) * xgap2);
        plotPixel(xpxl2, iy+1, wu_frac(yend2)  * xgap2);
    } else {
        // Regular main-loop column
        int iy = wu_ipart(intery);
        plotPixel(currentX, iy,   wu_rfrac(intery));
        plotPixel(currentX, iy+1, wu_frac(intery));
        intery += gradient;
    }
    currentX++;
    currentStep++;
}

// ---------------------------------------------------------------
//  init()
// ---------------------------------------------------------------
void XiaolinWuLine::init(int x0, int y0, int x1, int y1) {
    ox0=x0; oy0=y0; ox1=x1; oy1=y1;
    path.clear(); currentStep=0;

    // Determine steep
    steep = std::abs(y1-y0) > std::abs(x1-x0);

    float fx0, fy0, fx1, fy1;
    if (steep) { fx0=(float)y0; fy0=(float)x0; fx1=(float)y1; fy1=(float)x1; }
    else       { fx0=(float)x0; fy0=(float)y0; fx1=(float)x1; fy1=(float)y1; }

    if (fx0 > fx1) { std::swap(fx0,fx1); std::swap(fy0,fy1); }

    fpx0=fx0; fpy0=fy0; fpx1=fx1; fpy1=fy1;

    float dx = fx1-fx0, dy = fy1-fy0;
    gradient = (dx == 0.0f) ? 1.0f : dy/dx;

    // ---- First endpoint ----
    int   xend = wu_round(fx0);
    float yend = fy0 + gradient * (xend - fx0);
    float xgap = wu_rfrac(fx0 + 0.5f);
    xpxl1 = xend; yend1 = yend; xgap1 = xgap;

    // ---- Last endpoint ----
    xend = wu_round(fx1);
    yend = fy1 + gradient * (xend - fx1);
    xgap = wu_frac(fx1 + 0.5f);
    xpxl2 = xend; yend2 = yend; xgap2 = xgap;

    // intery for the column AFTER xpxl1
    intery     = yend1 + gradient;
    currentX   = xpxl1;
    totalSteps = xpxl2 - xpxl1 + 1;

    lastState = AlgoState();
    lastState.totalSteps    = totalSteps;
    lastState.pixelsPerStep = 2;
}

// ---------------------------------------------------------------
//  step() — advances one column and records full calculation
// ---------------------------------------------------------------
void XiaolinWuLine::step() {
    if (isFinished()) return;

    // Capture state BEFORE advancing
    int   preX     = currentX;
    float preIntery = intery;

    // === Execute one column ===
    lastState.calcLines.clear();
    lastState.hasCalculation  = true;
    lastState.pixelsPerStep   = 2;

    if (preX == xpxl1) {
        // ---- First endpoint ----
        int   iy = wu_ipart(yend1);
        float i1 = wu_rfrac(yend1) * xgap1;
        float i2 = wu_frac(yend1)  * xgap1;
        plotPixel(xpxl1, iy,   i1);
        plotPixel(xpxl1, iy+1, i2);

        // Build calculation
        lastState.calcLines.push_back("--- Step Type: First Endpoint ---");
        lastState.calcLines.push_back("  X column = xpxl1 = round("+f4(fpx0)+") = "+si(xpxl1));
        lastState.calcLines.push_back("  yend  = y0 + grad*(xpxl1-x0)");
        lastState.calcLines.push_back("       = "+f4(fpy0)+" + "+f4(gradient)+"*("+si(xpxl1)+"-"+f4(fpx0)+")");
        lastState.calcLines.push_back("       = "+f4(yend1));
        lastState.calcLines.push_back("  xgap = rfrac(x0+0.5) = rfrac("+f4(fpx0+0.5f)+") = "+f4(xgap1));
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("--- Intensity Calculation ---");
        lastState.calcLines.push_back("  ipart(yend) = "+si(iy)+"  (lower row)");
        lastState.calcLines.push_back("  rfrac(yend) = "+f4(wu_rfrac(yend1))+"  (how close to lower row)");
        lastState.calcLines.push_back("  i1 = rfrac(yend)*xgap = "+f4(wu_rfrac(yend1))+"*"+f4(xgap1)+" = "+f4(i1)+" ("+pct(i1)+")");
        lastState.calcLines.push_back("  frac(yend) = "+f4(wu_frac(yend1)));
        lastState.calcLines.push_back("  i2 = frac(yend)*xgap  = "+f4(wu_frac(yend1))+"*"+f4(xgap1)+" = "+f4(i2)+" ("+pct(i2)+")");

        int px1,py1,px2,py2;
        if(steep){px1=iy;py1=xpxl1;px2=iy+1;py2=xpxl1;}
        else     {px1=xpxl1;py1=iy;px2=xpxl1;py2=iy+1;}
        lastState.currentPixelInfo = ">> ("+si(px1)+","+si(py1)+") i="+f4(i1)
                                   + "   ("+si(px2)+","+si(py2)+") i="+f4(i2);

    } else if (preX == xpxl2) {
        // ---- Last endpoint ----
        int   iy = wu_ipart(yend2);
        float i1 = wu_rfrac(yend2) * xgap2;
        float i2 = wu_frac(yend2)  * xgap2;
        plotPixel(xpxl2, iy,   i1);
        plotPixel(xpxl2, iy+1, i2);

        lastState.calcLines.push_back("--- Step Type: Last Endpoint ---");
        lastState.calcLines.push_back("  X column = xpxl2 = round("+f4(fpx1)+") = "+si(xpxl2));
        lastState.calcLines.push_back("  yend  = y1 + grad*(xpxl2-x1)");
        lastState.calcLines.push_back("       = "+f4(fpy1)+" + "+f4(gradient)+"*("+si(xpxl2)+"-"+f4(fpx1)+")");
        lastState.calcLines.push_back("       = "+f4(yend2));
        lastState.calcLines.push_back("  xgap = frac(x1+0.5) = frac("+f4(fpx1+0.5f)+") = "+f4(xgap2));
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("--- Intensity Calculation ---");
        lastState.calcLines.push_back("  ipart(yend) = "+si(iy));
        lastState.calcLines.push_back("  i1 = rfrac(yend)*xgap = "+f4(wu_rfrac(yend2))+"*"+f4(xgap2)+" = "+f4(i1)+" ("+pct(i1)+")");
        lastState.calcLines.push_back("  i2 = frac(yend)*xgap  = "+f4(wu_frac(yend2))+"*"+f4(xgap2)+" = "+f4(i2)+" ("+pct(i2)+")");

        int px1,py1,px2,py2;
        if(steep){px1=iy;py1=xpxl2;px2=iy+1;py2=xpxl2;}
        else     {px1=xpxl2;py1=iy;px2=xpxl2;py2=iy+1;}
        lastState.currentPixelInfo = ">> ("+si(px1)+","+si(py1)+") i="+f4(i1)
                                   + "   ("+si(px2)+","+si(py2)+") i="+f4(i2);

    } else {
        // ---- Regular main-loop column ----
        float  f  = wu_frac(preIntery);
        float  rf = wu_rfrac(preIntery);
        int    iy = wu_ipart(preIntery);
        plotPixel(preX, iy,   rf);
        plotPixel(preX, iy+1, f);

        lastState.calcLines.push_back("--- Step Type: Main Loop ---");
        lastState.calcLines.push_back("  X column  = "+si(preX));
        lastState.calcLines.push_back("  intery    = "+f4(preIntery)+"  (floating Y position)");
        lastState.calcLines.push_back("  next intery += gradient: "+f4(preIntery)+"+"+f4(gradient)+"="+f4(preIntery+gradient));
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("--- Intensity Calculation ---");
        lastState.calcLines.push_back("  ipart(intery) = floor("+f4(preIntery)+") = "+si(iy));
        lastState.calcLines.push_back("  frac(intery)  = "+f4(preIntery)+"-"+si(iy)+" = "+f4(f));
        lastState.calcLines.push_back("  rfrac(intery) = 1 - "+f4(f)+" = "+f4(rf));
        lastState.calcLines.push_back("");
        lastState.calcLines.push_back("  frac  tells us: how far ABOVE the lower row the line is");
        lastState.calcLines.push_back("  rfrac tells us: how close to the LOWER row");
        lastState.calcLines.push_back("");

        // Colour intensity bars (ASCII visualization)
        int barLow  = (int)(rf * 20.0f);
        int barHigh = (int)(f  * 20.0f);
        std::string barL(barLow,  '#'); barL  += std::string(20-barLow,  '.');
        std::string barH(barHigh, '#'); barH  += std::string(20-barHigh, '.');

        int px1,py1,px2,py2;
        if(steep){px1=iy;py1=preX;px2=iy+1;py2=preX;}
        else     {px1=preX;py1=iy;px2=preX;py2=iy+1;}

        lastState.calcLines.push_back("  Lower pixel ("+si(px1)+","+si(py1)+"): rfrac = "+f4(rf)+" ("+pct(rf)+") ["+barL+"]");
        lastState.calcLines.push_back("  Upper pixel ("+si(px2)+","+si(py2)+"): frac  = "+f4(f) +" ("+pct(f) +") ["+barH+"]");
        lastState.calcLines.push_back("  Note: intensities always sum to 1.0");

        intery += gradient;

        lastState.currentPixelInfo = ">> Lower:("+si(px1)+","+si(py1)+") i="+f4(rf)
                                   + "  Upper:("+si(px2)+","+si(py2)+") i="+f4(f);
    }

    currentX++;
    currentStep++;
    lastState.currentStep = currentStep;
    lastState.totalSteps  = totalSteps;
}

// ---------------------------------------------------------------
//  stepK() — silent
// ---------------------------------------------------------------
void XiaolinWuLine::stepK(int k) {
    for (int i=0; i<k && !isFinished(); ++i) doStep();
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
    lastState.currentPixelInfo = "";
}

// ---------------------------------------------------------------
//  runToCompletion() — silent
// ---------------------------------------------------------------
void XiaolinWuLine::runToCompletion() {
    while (!isFinished()) doStep();
    lastState.hasCalculation = false;
    lastState.currentStep    = currentStep;
    lastState.totalSteps     = totalSteps;
    lastState.calcLines.clear();
}

void XiaolinWuLine::reset() { init(ox0,oy0,ox1,oy1); }
bool XiaolinWuLine::isFinished() const { return currentStep >= totalSteps; }
std::vector<Pixel> XiaolinWuLine::getHighlightedPixels() const { return path; }
AlgoState XiaolinWuLine::getCurrentState() const { return lastState; }
std::string XiaolinWuLine::getName() const { return "Xiaolin Wu's Anti-Aliased Line"; }

std::vector<std::string> XiaolinWuLine::getInitInfo() const {
    std::string sw = steep ? "YES  ->  X & Y swapped for computation"
                           : "NO   ->  no swap needed";
    float dx = fpx1-fpx0, dy = fpy1-fpy0;
    float g  = (dx==0.0f) ? 1.0f : dy/dx;
    return {
        "Original: ("+si(ox0)+","+si(oy0)+") -> ("+si(ox1)+","+si(oy1)+")",
        "Steep: "+sw,
        "Processed: ("+f2(fpx0)+","+f2(fpy0)+") -> ("+f2(fpx1)+","+f2(fpy1)+")",
        "gradient = dy/dx = "+f4(dy)+"/"+f4(dx)+" = "+f4(g),
        "xpxl1="+si(xpxl1)+"  yend1="+f4(yend1)+"  xgap1="+f4(xgap1),
        "xpxl2="+si(xpxl2)+"  yend2="+f4(yend2)+"  xgap2="+f4(xgap2),
        "Main loop: x = "+si(xpxl1)+" to "+si(xpxl2)+"  (",si(totalSteps)+" columns)"
    };
}

std::vector<std::string> XiaolinWuLine::getCurrentVars() const {
    float f  = wu_frac(intery);
    float rf = 1.0f - f;
    int   iy = wu_ipart(intery);
    return {
        "Current X  : "+si(currentX)+" / "+si(xpxl2),
        "intery     : "+f4(intery)+"  (float Y)",
        "ipart(y)   : "+si(iy)+"  (lower pixel row)",
        "frac(y)    : "+f4(f)+"  -> upper pixel intensity",
        "rfrac(y)   : "+f4(rf)+" -> lower pixel intensity",
        "gradient   : "+f4(gradient),
        "steep      : "+(steep?std::string("YES"):std::string("NO")),
        "Plotted    : "+si((int)path.size())+" pixel-ops (2 per step)",
    };
}

std::string XiaolinWuLine::getTheory() const {
    return
        "Xiaolin Wu's Anti-Aliased Line Algorithm\n"
        "=========================================\n\n"
        "WHAT IS IT?\n"
        "  Published by Xiaolin Wu in 1991.\n"
        "  Produces smooth, anti-aliased lines by plotting\n"
        "  TWO pixels per column with fractional intensities\n"
        "  that together always sum to 1.0.\n\n"
        "THE PROBLEM WITH BRESENHAM:\n"
        "  Bresenham plots exactly ONE pixel per column, which\n"
        "  produces a hard, jagged 'staircase' appearance\n"
        "  (aliasing). The eye perceives the steps clearly.\n\n"
        "XIAOLIN WU'S SOLUTION:\n"
        "  Instead of picking one pixel, plot BOTH the lower\n"
        "  and upper neighbours with complementary intensities\n"
        "  based on how close the ideal line passes to each.\n\n"
        "KEY CONCEPTS:\n"
        "  frac(y)  = fractional part of y  = y - floor(y)\n"
        "  rfrac(y) = 1 - frac(y)           = reverse fraction\n"
        "  ipart(y) = floor(y)              = integer part\n\n"
        "MAIN LOOP (for each x column):\n"
        "  Lower pixel: (x, ipart(intery))   intensity = rfrac(intery)\n"
        "  Upper pixel: (x, ipart(intery)+1) intensity = frac(intery)\n"
        "  intery += gradient  (advance by slope each step)\n\n"
        "ENDPOINT HANDLING:\n"
        "  The two endpoints are handled separately using xgap\n"
        "  (the fractional coverage at the line's start/end)\n"
        "  to blend them correctly with the background.\n\n"
        "VISUAL EXAMPLE  (at one column):\n"
        "  If ideal y = 3.7:\n"
        "    Lower pixel (y=3): intensity = rfrac(3.7) = 0.3  (30%)\n"
        "    Upper pixel (y=4): intensity = frac(3.7)  = 0.7  (70%)\n"
        "  The eye perceives the line as smoother than Bresenham.\n\n"
        "STEEP LINES:\n"
        "  When |dy|>|dx|, axes are swapped internally so\n"
        "  the algorithm always steps along the major axis.\n"
        "  Co-ordinates are un-swapped when plotting.\n\n"
        "ADVANTAGES:\n"
        "+   Visually smooth — no aliasing staircase\n"
        "+   Only uses floating-point frac/rfrac operations\n"
        "+   Handles all slopes and directions\n\n"
        "DISADVANTAGES:\n"
        "-   Slightly more expensive than Bresenham\n"
        "-   Requires blending support in the renderer\n"
        "-   Intensity varies with background colour\n\n"
        "TIME COMPLEXITY:  O( max(|dx|,|dy|) )\n"
        "SPACE COMPLEXITY: O( 2n ) — two pixels per column\n";
}