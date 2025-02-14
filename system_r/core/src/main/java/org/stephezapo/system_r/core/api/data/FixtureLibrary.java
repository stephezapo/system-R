package org.stephezapo.system_r.core.api.data;

import java.util.ArrayList;
import java.util.List;

public class FixtureLibrary
{
    private List<String> manufacturers = new ArrayList<>();

    public class Fixture
    {
        private String name;
        private String manufacturer;
        private List<FixtureMode> modes = new ArrayList<>();
    }

    public class FixtureMode
    {
        private String name;
        private String description;
        private String version;
    }
}