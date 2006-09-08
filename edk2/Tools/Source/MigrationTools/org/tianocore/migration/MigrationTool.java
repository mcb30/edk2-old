/** @file
 
 Copyright (c) 2006, Intel Corporation
 All rights reserved. This program and the accompanying materials
 are licensed and made available under the terms and conditions of the BSD License
 which accompanies this distribution.  The full text of the license may be found at
 http://opensource.org/licenses/bsd-license.php
 
 THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 
 **/
package org.tianocore.migration;

import java.io.File;
import java.util.*;

import javax.swing.JFileChooser;

public class MigrationTool {
    public static UI ui = null;
    public static Database db = null;

    public static String MIGRATIONCOMMENT = "//%@//";

    public static boolean printModuleInfo = false;
    public static boolean doCritic = false;
    public static boolean defaultoutput = false;
    
    public static final HashMap<ModuleInfo, String> ModuleInfoMap = new HashMap<ModuleInfo, String>();

    private static final void mainFlow(ModuleInfo mi) throws Exception {

        ModuleReader.ModuleScan(mi);
        
        //MigrationTool.ui.yesOrNo("go on replace?");
        SourceFileReplacer.fireAt(mi);    // some adding library actions are taken here,so it must be put before "MsaWriter"

        //MigrationTool.ui.yesOrNo("go on show?");
        // show result
        if (MigrationTool.printModuleInfo) {
            MigrationTool.ui.println("\nModule Information : ");
            MigrationTool.ui.println("Entrypoint : " + mi.entrypoint);
            show(mi.protocol, "Protocol : ");
            show(mi.ppi, "Ppi : ");
            show(mi.guid, "Guid : ");
            show(mi.hashfuncc, "call : ");
            show(mi.hashfuncd, "def : ");
            show(mi.hashEFIcall, "EFIcall : ");
            show(mi.hashnonlocalmacro, "macro : ");
            show(mi.hashnonlocalfunc, "nonlocal : ");
            show(mi.hashr8only, "hashr8only : ");
        }

        //MigrationTool.ui.yesOrNo("go on msawrite?");
        new MsaWriter(mi).flush();
        //MigrationTool.ui.yesOrNo("go on critic?");

        if (MigrationTool.doCritic) {
            Critic.fireAt(ModuleInfoMap.get(mi) + File.separator + "Migration_" + mi.modulename);
        }

        //MigrationTool.ui.yesOrNo("go on delete?");
        Common.deleteDir(mi.modulepath + File.separator + "temp");

        MigrationTool.ui.println("Errors Left : " + MigrationTool.db.error);
        MigrationTool.ui.println("Complete!");
        //MigrationTool.ui.println("Your R9 module was placed here: " + mi.modulepath + File.separator + "result");
        //MigrationTool.ui.println("Your logfile was placed here: " + mi.modulepath);
    }

    private static final void show(Set<String> hash, String show) {
        MigrationTool.ui.println(show + hash.size());
        MigrationTool.ui.println(hash);
    }

    private static final String assignOutPutPath(String inputpath) {
        if (MigrationTool.defaultoutput) {
            return inputpath.replaceAll(Common.strseparate, "$1");
        } else {
            return MigrationTool.ui.getFilepath("Please choose where to place the output module", JFileChooser.DIRECTORIES_ONLY);
        }
    }
    
    public static final void seekModule(String filepath) throws Exception {
        if (ModuleInfo.isModule(filepath)) {
            ModuleInfoMap.put(new ModuleInfo(filepath), assignOutPutPath(filepath));
        }
    }

    public static final void startMigrateAll(String path) throws Exception {
        MigrationTool.ui.println("Project Migration");
        MigrationTool.ui.println("Copyright (c) 2006, Intel Corporation");
        Common.toDoAll(path, MigrationTool.class.getMethod("seekModule", String.class), null, null, Common.DIR);
        
        Iterator<ModuleInfo> miit = ModuleInfoMap.keySet().iterator();
        while (miit.hasNext()) {
            mainFlow(miit.next());
        }
        
        ModuleInfoMap.clear();
    }

    public static void main(String[] args) throws Exception {
        ui = FirstPanel.getInstance();
        db = Database.getInstance();
    }
}
