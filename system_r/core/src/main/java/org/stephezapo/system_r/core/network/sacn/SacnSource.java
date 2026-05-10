package org.stephezapo.system_r.core.network.sacn;

import java.net.InetAddress;
import java.util.Arrays;
import java.util.Collections;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

public class SacnSource
{
    private final byte[]       cid;
    private final String       cidString;
    private final InetAddress  address;
    private volatile String    sourceName;
    private volatile long      lastSeen;
    private final Set<Integer> universes = ConcurrentHashMap.newKeySet();

    public SacnSource(byte[] cid, InetAddress address, String sourceName)
    {
        this.cid        = Arrays.copyOf(cid, cid.length);
        this.cidString  = cidToString(cid);
        this.address    = address;
        this.sourceName = sourceName;
        this.lastSeen   = System.currentTimeMillis();
    }

    public void updateLastSeen()
    {
        this.lastSeen = System.currentTimeMillis();
    }

    public void updateSourceName(String name)
    {
        this.sourceName = name;
    }

    public void addUniverse(int universe)
    {
        universes.add(universe);
    }

    public void setUniverses(Iterable<Integer> list)
    {
        universes.clear();
        list.forEach(universes::add);
    }

    public byte[]      getCid()        { return Arrays.copyOf(cid, cid.length); }
    public String      getCidString()  { return cidString; }
    public InetAddress getAddress()    { return address; }
    public String      getSourceName() { return sourceName; }
    public long        getLastSeen()   { return lastSeen; }
    public Set<Integer> getUniverses() { return Collections.unmodifiableSet(universes); }

    public static String cidToString(byte[] cid)
    {
        var sb = new StringBuilder();
        for (int i = 0; i < cid.length; i++)
        {
            if (i == 4 || i == 6 || i == 8 || i == 10) sb.append('-');
            sb.append(String.format("%02x", cid[i] & 0xFF));
        }
        return sb.toString();
    }

    @Override
    public String toString()
    {
        return "SacnSource{cid=" + cidString + ", ip=" + address.getHostAddress()
               + ", name='" + sourceName + "', universes=" + universes + "}";
    }
}
