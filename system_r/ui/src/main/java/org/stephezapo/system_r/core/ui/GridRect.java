package org.stephezapo.system_r.core.ui;

public class GridRect
{
    private int x;
    private int y;
    private int w;
    private int h;

    public GridRect(int x, int y, int w, int h)
    {
        this.x = x;
        this.y = y;
        this.w = w;
        this.h = h;
    }

    public boolean contains(GridPoint point)
    {
        return (point.getX() >= this.x && point.getX() < this.x + this.w &&
            point.getY() >= this.y && point.getY() < this.y + this.h);
    }

    public int getX()
    {
        return x;
    }

    public void setX(int x)
    {
        this.x = x;
    }

    public int getY()
    {
        return y;
    }

    public void setY(int y)
    {
        this.y = y;
    }

    public int getW()
    {
        return w;
    }

    public void setW(int w)
    {
        this.w = w;
    }

    public int getH()
    {
        return h;
    }

    public void setH(int h)
    {
        this.h = h;
    }

    public String toString()
    {
        return "[x:" + x + " y:" + y + " w:" + w + " h:" + h + "]";
    }
}