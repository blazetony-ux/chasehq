package main

import (
 "encoding/json"
 "os"
 "path/filepath"
 "strings"
 "testing"
)
func TestRC5WorkbenchNextStepsSynced(t *testing.T) {
 root:=filepath.Join("..","..")
 b,err:=os.ReadFile(filepath.Join(root,"games","chasehq","next-steps.json"));if err!=nil{t.Fatal(err)}
 var data struct{Schema string `json:"schema"`;Items []struct{Title string `json:"title"`;Status string `json:"status"`;Detail string `json:"detail"`} `json:"items"`}
 if err=json.Unmarshal(b,&data);err!=nil{t.Fatal(err)}
 if data.Schema!="chq-next-steps-v1"||len(data.Items)<9{t.Fatal("stale Workbench Next Steps")}
 blob:=string(b)
 for _,phrase:=range []string{"0x3B84","6144","832","BEHAVIOUR UNDERSTOOD","NOT"}{
  if !strings.Contains(strings.ToUpper(blob),strings.ToUpper(phrase)){t.Errorf("next steps missing %q",phrase)}
 }
 for _,x:=range data.Items {if x.Title==""||x.Status==""||x.Detail==""{t.Fatal("empty next step")}}
 b,err=os.ReadFile(filepath.Join(root,"Start-ChaseHQWeb.ps1"));if err!=nil{t.Fatal(err)}
 if !strings.Contains(string(b),"/api/v1/next-steps")||!strings.Contains(string(b),"$nextStepsPath") {t.Fatal("Workbench Next Steps not connected to source JSON")}
}
func TestRC5NativeCannotReplace(t *testing.T){
 root:=filepath.Join("..","..")
 p,err:=os.ReadFile(filepath.Join(root,"src","native_execution.h"));if err!=nil{t.Fatal(err)}
 if !strings.Contains(string(p),"return mode == Mode::Observe"){t.Fatal("native replacement safety guard changed")}
}
