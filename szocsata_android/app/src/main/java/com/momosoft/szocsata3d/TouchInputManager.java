package com.momosoft.szocsata3d;

import android.graphics.Point;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;

public class TouchInputManager {

    public Point m_FingerPos0 = new Point();
    public Point m_FingerPos1 = new Point();
    private boolean m_MultyTouch = false;
    private double Distance = 0;

    public MainActivity m_ParentActivity = null;

    public TouchInputManager(MainActivity parentActivity) {
        m_ParentActivity = parentActivity;
    }

    View.OnTouchListener m_TouchInputListener = new View.OnTouchListener () {

        @Override
        public boolean onTouch(View v, MotionEvent event) {

            int Action = event.getActionMasked();

            if (Action == MotionEvent.ACTION_DOWN) {
                Log.i("fos", "finger one down : " + 0);
                m_ParentActivity.hideStatusBar();
                int x = (int)event.getX(0);
                int y = (int)event.getY(0);
                m_FingerPos0.x = x;
                m_FingerPos0.y = y;

                if (!m_MultyTouch)
                    m_ParentActivity.HandleTouchEvent(x, y);

                return true;
            }

            if (Action == MotionEvent.ACTION_POINTER_DOWN) {
                Log.i("fos", "finger 2 down : " + 1);
                m_MultyTouch = true;
                int x = (int)event.getX(1);
                int y = (int)event.getY(1);
                m_FingerPos1.x = x;
                m_FingerPos1.y = y;
                Distance = Math.sqrt((double)((m_FingerPos0.x - m_FingerPos1.x) * (m_FingerPos0.x - m_FingerPos1.x) + (m_FingerPos0.y - m_FingerPos1.y) * (m_FingerPos0.y - m_FingerPos1.y)));
                Log.i("dist", "start Distance : " + Distance);
                m_ParentActivity.HandleMultyTouchStartEvent(m_FingerPos0.x, m_ParentActivity.GetGLViewSize().y - m_FingerPos0.y, m_FingerPos1.x, m_ParentActivity.GetGLViewSize().y - m_FingerPos1.y);
                return  true;
            }

            if (Action == MotionEvent.ACTION_UP) {
                Log.i("fos", "finger 1 up : " + 1);
                if (m_MultyTouch) {
                    m_ParentActivity.HandleMultyTouchEndEvent();
                    m_MultyTouch = false;
                }
                else {
                    int x = (int)event.getX(0);
                    int y = (int)event.getY(0);
                    Log.i("fos", "HandleRelease x : " + x + " y : " + y );
                    m_ParentActivity.HandleReleaseEvent(x, y);
                }
                return true;
            }

            if (Action == MotionEvent.ACTION_POINTER_UP) {
                Log.i("fos", "finger 2 up : " + 1);
                m_ParentActivity.HandleMultyTouchEndEvent();
                m_MultyTouch = false;
                return true;
            }

            if (Action == MotionEvent.ACTION_MOVE) {
                //                                                 Log.i("fos", "finger move : " + event.getPointerCount() + " kk " + m_MultyTouch);
                if (m_MultyTouch) {

                    int pointerCount = event.getPointerCount();
                    int dif = 0;

                    for(int i = 0; i < pointerCount; ++i)
                    {
                        int pointerIndex = i;
                        int pointerId = event.getPointerId(pointerIndex);
                        Log.d("pointer id - move",Integer.toString(pointerId));
                        if(pointerId == 0)
                        {
                            dif += Math.abs(m_FingerPos0.x - (int)event.getX(pointerIndex)) + Math.abs(m_FingerPos0.y - (int)event.getX(pointerIndex));
                            m_FingerPos0.x = (int)event.getX(pointerIndex);
                            m_FingerPos0.y = (int)event.getY(pointerIndex);
                            Log.i("fos", "finger 0 moved :  x - " + m_FingerPos0.x + " y - " + m_FingerPos0.y);
                        }
                        if(pointerId == 1)
                        {
                            dif += Math.abs(m_FingerPos1.x - (int)event.getX(pointerIndex)) + Math.abs(m_FingerPos1.y - (int)event.getX(pointerIndex));
                            m_FingerPos1.x = (int)event.getX(pointerIndex);
                            m_FingerPos1.y = (int)event.getY(pointerIndex);
                            Log.i("fos", "finger 1 moved :  x - " + m_FingerPos1.x + " y - " + m_FingerPos1.y);
                        }
                    }

                    //if (dif > 4) {
                    double Dist = Math.sqrt((double)((m_FingerPos0.x - m_FingerPos1.x) * (m_FingerPos0.x - m_FingerPos1.x) + (m_FingerPos0.y - m_FingerPos1.y) * (m_FingerPos0.y - m_FingerPos1.y)));
                    Log.i("dist", "Distance : " + (Dist - Distance));
                    Distance = Dist;
                    m_ParentActivity.HandleMultyTouchEvent(m_FingerPos0.x, m_ParentActivity.GetGLViewSize().y - m_FingerPos0.y, m_FingerPos1.x, m_ParentActivity.GetGLViewSize().y - m_FingerPos1.y);
                    //}
                }

                else {
                    int x = (int)event.getX(0);
                    int y = (int)event.getY(0);
                    Log.i("fos", "HandleDrag x : " + x + " y : " + y );
                    m_ParentActivity.HandleDragEvent(x, y);
                }

                return true;
            }

            return v.onTouchEvent(event);
        }
    };
}
