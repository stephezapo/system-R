package org.zapo.systemr.ui.controller;

import javafx.event.ActionEvent;
import javafx.fxml.FXML;
import org.apache.thrift.TException;
import org.zapo.systemr.core.Core;

public class MainWindowController {

    @FXML
    private void test(ActionEvent event)
    {
        System.out.println("lollolol");
        try {
            Core.Get().getAPI().login("User", "pass");
        } catch (TException e) {
            e.printStackTrace();
        }
    }
}
