package org.stephezapo.system_r.api.fixture.library;

import java.io.Serializable;

public class LibraryState implements Serializable
{
    public enum State
    {
        EMPTY,
        READY,
        IMPORTING
    }

    private State state;
    private short progress;

    public LibraryState()
    {

    }

    public void setState(State state)
    {
        this.state = state;
    }

    public State getState()
    {
        return this.state;
    }

    public void setProgress(short progress)
    {
        this.progress = (short)Math.min(Math.max(0, progress), 100);

        this.state = progress < 100 ? State.IMPORTING : State.READY;
    }

    public short getProgress()
    {
        return this.progress;
    }
}
