#include <cstdio>
#include <sstream>
#include <cstdlib>
#include <TTree.h>
#include <TFile.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <TH1F.h>
#include <TH2.h>
#include <TCanvas.h>
#include <TApplication.h>
#include <TLegend.h>
#include <TLegendEntry.h>
#include <TPaveStats.h>
#include <TLatex.h>
#include <TROOT.h>
#include <thread> // Add this line for std::this_thread::sleep_for
#include <fstream>
#include <ctime>
#include <string> //
// #include <filesystem>
#include <dirent.h>
#include <unistd.h>
#include <termios.h>
#include <fnmatch.h> // Channelconfig
#include "TTree.h"
#include "TTreeReader.h"
#include "TDirectory.h"
#include <TBranch.h>
#include <TMath.h>
#include <cstdlib>
#include <TStyle.h> // Include the header file for gStyle
#include "TCutG.h"

// CAEN BOARDS AND CHNANELS YYYYYYYYYYYYOOOOOOOOOOOOOOOUUUUUUUUUUUUUUUURRRRRRRRRRRRRRRRRRR EDITS

// BAORD 0
// T1 ch0
// T2 ch1
// X1 ch0
// X2 ch1
// Y1 ch0
// Y2 ch1
//int gates[] = {-30000,	10000,	15000,	35000,	-10000,	50000,	-16000,	6000
//
//
//};-10000	50000	-16000	6000	-30000	10000	10000	35000

int gates[] = {	-10000,	50000,	-16000,	-4000,	-30000,	10000,	10000,	35000

};

// CEJ
// Maybe we can define some variables for plotting (gates etc)
// we can also set up a waiter for the timesorting and this event building to
// produce files automatically.


int plotter =0;// plotter 1 macht die bilder schoener aber blod zum arbeiten
std::string Ordner = "./";
std::string Runname = "run_9";        // no
std::string dataformat = "UNFILTERED"; // no/
//std::string dataformat = ""; // no/

// CEJ this is old stuff
int refreshcounter = 20000;            // wie oft soll der plotter geupdatet werden
int sleeptimer = 1;                    // in sekunden
std::string checkfile = "Data_run*.root";

// structure erstellen
struct Entry
{
  UShort_t Channel;
  ULong64_t Timestamp;
  UShort_t Energy;
  UShort_t Board;
};


int countRootFilesInFolder(const std::string &folderPath)
{
  int count = 0;
  DIR *dir = opendir(folderPath.c_str());

  if (dir != nullptr)
  {
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr)
    {
      std::string filename = entry->d_name;
      if ((filename.size() >= 9 && filename.substr(0, 4) == "Data") && (filename.substr(filename.size() - 5) == ".root"))
      // if ( filename.substr(filename.size() - 5) == ".root")

      {
        count++;
      }
    }
    closedir(dir);
  }
  return count;
}

int main()
{
  std::string folderPath = Ordner + Runname + "/" + dataformat;
  std::string SfolderPath = folderPath + "/timesorted";

  std::string baseFileName = Ordner + Runname + "/" + dataformat + "/Data_" + Runname;
  std::string SfileName = SfolderPath + "/Data_" + Runname;

  std::cout << ": " << folderPath << std::endl;

  DIR *dir;
  struct dirent *entry;

  // zwischenspeicher fuereventbuilding
  ULong64_t ttrigger0; // T1 hat leider der CAEN stempel geklaut
  Long64_t time_since_last_trigger; 
  ULong64_t T2;
  Long64_t X1;
  Long64_t X2;
  Long64_t Y1;
  Long64_t Y2;
  Long64_t X3;
  Long64_t X4;
  Long64_t Y3;
  Long64_t Y4;
  Long64_t tdiff;
  Long64_t xdiff;
  Long64_t ydiff;
  
  // messy, will tidy later
  int maxFileCount = 0;
  int ch1counter = 0;
  int ch2counter = 0;
  int ch3counter = 0;
  int ch4counter = 0;
  int ch5counter = 0;
  int ch6counter = 0;
  int ch7counter = 0;
  int ch8counter = 0;
  int ch9counter = 0;
  int ch10counter = 0;
  int fileCount = 0;
  bool foundMatchingFile = false;
  int reruns = 0;
  
  // Output root file
  std::string outputFileName = Runname + "_output.root";
  const char *outputFileNameCStr = outputFileName.c_str();
  TFile* of = new TFile(outputFileNameCStr, "RECREATE");
  
  // Directories are nice!
  TDirectory* dir_dssd = gDirectory->mkdir("DSSD_Energy");
  
  // ::: Define Canvases ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TCanvas* c_delta_xy_MCP1 = new TCanvas("c_delta_xy_MCP1", "MCP1 Delta X and Y", 1920, 480);
  TCanvas* c_delta_xy_MCP2 = new TCanvas("c_delta_xy_MCP2", "MCP2 Delta X and Y", 1920, 480);
  c_delta_xy_MCP1->Divide(1, 2);
  c_delta_xy_MCP2->Divide(1, 2);
  
  TCanvas* c_heatmap_MCP1 = new TCanvas("c_heatmap_MCP1", "DDL Heatmap MCP 1", 1350, 1080);
  TCanvas* c_heatmap_MCP2 = new TCanvas("c_heatmap_MCP2", "DDL Heatmap MCP 2", 1350, 1080);
  // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  
  // ::: Define Histograms :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TH1F* h1_mcp_tdiff = new TH1F("h1_mcp_tdiff", "MCP TDiff Distribution", 10000, -400000, 0);
  
  TH1F* h1_mcp1_event_dt = new TH1F("h1_mcp1_event_dt", "TDiff between MCP1 triggers", 1000, 100000, 100000000);

  TH2F* h2_DDL_heatmap_MCP1 = new TH2F("h2_DDL_heatmap_MCP1", "DDL Heatmap MCP1;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  TH2F* h2_DDL_heatmap_MCP2 = new TH2F("h2_DDL_heatmap_MCP2", "DDL Heatmap MCP2;#DeltaX in ns;#DeltaY in ns", 2000, -300001, 300001, 2000, -300001, 300001);
  h2_DDL_heatmap_MCP1->SetContour(40);
  
  TH1F* h1_deltaX_mcp1 = new TH1F("h1_deltaX_mcp1","MCP 1 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp1 = new TH1F("h1_deltaY_mcp1","MCP 1 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);
  TH1F* h1_deltaX_mcp2 = new TH1F("h1_deltaX_mcp2","MCP 2 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp2 = new TH1F("h1_deltaY_mcp2","MCP 2 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);

  // gate on what exactly?
  TH1F* h1_deltaX_mcp1_gated = new TH1F("h1_deltaX_mcp1_gated","MCP 1 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp1_gated = new TH1F("h1_deltaY_mcp1_gated","MCP 1 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);
  TH1F* h1_deltaX_mcp2_gated = new TH1F("h1_deltaX_mcp2_gated","MCP 2 xgate; #DeltaX in ns;Time (ns);Counts", 1000, gates[0], gates[1]);
  TH1F* h1_deltaY_mcp2_gated = new TH1F("h1_deltaY_mcp2_gated","MCP 2 ygate; #DeltaY in ns;Time (ns);Counts", 1000, gates[4], gates[5]);
  
  TH1F* h1_labr_energy = new TH1F("h1_labr_energy", "LaBr Energy", 1000, 0, 2000);
  
  TH1F* h1_germanium_energy = new TH1F("h1_germanium_energy", "Germanium Energy", 1000, 0, 2000);
  
  dir_dssd->cd();
  TH1F* h1_dssd_energy[64];
  for (int i = 0; i < 64; i++) h1_dssd_energy[i] = new TH1F(Form("h1_dssd_energy_%i", i), Form("DSSD Energy Channel %i", i), 1000, 0, 6000);
  // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  
  // ::: Gates :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
  TCutG *mcp1_cut = new TCutG("mcp1_cut", 12);
	mcp1_cut->SetVarX("xdiff");
	mcp1_cut->SetVarY("ydiff");
	// Tb166->SetPoint(0,2.54322,65.5306);
	// Tb166->SetPoint(1,2.54062,65.4287);
	// Tb166->SetPoint(2,2.53911,65.2602);
	// Tb166->SetPoint(3,2.54057,65.0954);
	// Tb166->SetPoint(4,2.54487,64.9917);
	// Tb166->SetPoint(5,2.54944,64.9694);
	// Tb166->SetPoint(6,2.55182,65.112);
	// Tb166->SetPoint(7,2.55182,65.312);
  // Tb166->SetPoint(8,2.54981,65.4528);
	// Tb166->SetPoint(9,2.5435,65.5546);
	// Tb166->SetPoint(10,2.54341,65.5306);
	// Tb166->SetPoint(11,2.54322,65.5306); 

  TCutG *mcp2_cut = new TCutG("mcp2_cut", 12);
	mcp2_cut->SetVarX("xdiff");
	mcp2_cut->SetVarY("ydiff");
  // :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


  const char *filepathCStr = baseFileName.c_str(); // umwandlung von string to c_string (vill)
  const char *checkfilechar = checkfile.c_str();

  int p = 0;
  p = countRootFilesInFolder(folderPath.c_str());
  DIR *directory = opendir(folderPath.c_str());
  if (directory == nullptr)
  {
    std::cerr << "Failed to open directory." << std::endl;
    return 1;
  }
  for (int i = 0; i < p; i++) // hier sollte p stehen
  {
    std::stringstream SfilePathStream;
    SfilePathStream << SfileName;
    if (i > 0)
    { // kondition das der erste file richtige gelesen wird
      SfilePathStream << "_" << i;
    }
    
    std::string SfilePath = SfilePathStream.str() + "_timesorted.root";
    const char *SfilepathCStr = SfilePath.c_str();
    TFile *file = TFile::Open(SfilepathCStr);
    std::cout << "Open file: " << SfilepathCStr << std::endl;

    TTree *tree = dynamic_cast<TTree *>(file->Get("SortedData"));
    
    // Initialise TreeReader
	TTreeReader tr(tree);
	
	// Retrieve Branches
	TTreeReaderValue<UShort_t> board(tr, "Board");
	TTreeReaderValue<UShort_t> channel(tr, "Channel");
	TTreeReaderValue<UShort_t> energy(tr, "Energy");
	TTreeReaderValue<ULong64_t> timestamp(tr, "Timestamp");
	
	std::cout << "Processing " << tree->GetEntries() << " entries." << std::endl;
	

  // more counters
	int completeEvent = 0;
	int incompleteEvent = 0;
  int completeMCP1 = 0;
  int completeMCP2 = 0;
	int noMCP2 = 0;

  int n_triggers = 0; // mpc1
  int n_x1 = 0;
  int n_x2 = 0;
  int n_y1 = 0;
  int n_y2 = 0;
  int n_t2 = 0; // mpc2
  int n_x3 = 0;
  int n_x4 = 0;
  int n_y3 = 0;
  int n_y4 = 0;
	
	while (tr.Next())
	{
    

		if (*channel == 0 && *board == 0)
		{
      //std::cout << "::: NEW TRIGGER :::::" << std::endl;
			// process previous event, fill histograms, etc
			if (ch1counter > 0)
			{
				int mcp1_sum = ch1counter + ch2counter + ch3counter + ch4counter + ch5counter;
				int mcp2_sum = ch6counter + ch7counter + ch8counter + ch9counter + ch10counter;
				
				if (ch6counter > 0)
				{
					tdiff = ttrigger0 - T2; // inverse?
					h1_mcp_tdiff->Fill(tdiff);
				}
				
				if (mcp1_sum == 5) // is this desired if we don't have mcp2 information?
				{
					xdiff = X1 - X2;
					ydiff = Y1 - Y2;
					h2_DDL_heatmap_MCP1->Fill(xdiff, ydiff);
					
					if (xdiff > gates[0] && xdiff < gates[1] && ydiff > gates[2] && ydiff < gates[3])
					{
					  h1_deltaX_mcp1->Fill(xdiff);
					}
					if (xdiff > gates[6] && xdiff < gates[7] && ydiff > gates[4] && ydiff < gates[5])
					{
					  h1_deltaY_mcp1->Fill(ydiff);
					}

          completeMCP1++;
				}
				
				if (mcp2_sum == 5)
				{				
					tdiff = ttrigger0 - T2; // inverse?
					h1_mcp_tdiff->Fill(tdiff);
					
					xdiff = X3 - X4;
					ydiff = Y3 - Y4;
					h2_DDL_heatmap_MCP2->Fill(xdiff, ydiff);
					
					if (xdiff > gates[0] && xdiff < gates[1] && ydiff > gates[2] && ydiff < gates[3])
					{
					  h1_deltaX_mcp2->Fill(xdiff);
					}
					if (xdiff > gates[6] && xdiff < gates[7] && ydiff > gates[4] && ydiff < gates[5])
					{
					  h1_deltaY_mcp2->Fill(ydiff);
				    }

            completeMCP2++;
				}
				
				if (mcp1_sum + mcp2_sum == 10) // 5 for test file, 10 with 2 MCPs!
				{
					completeEvent++;
					
					// gate on tdiff peak
					// if (tdiff > 1000 && tdiff < 2000) h1_gated_spectrum->Fill();
					
				}
				else incompleteEvent++;
			}

      if (ch1counter > 0) 
      {
        time_since_last_trigger = *timestamp - ttrigger0;
        h1_mcp1_event_dt->Fill(time_since_last_trigger);
      }
			
			
			// reset counters
			ch1counter = 0;
			ch2counter = 0;
		    ch3counter = 0;
			ch4counter = 0;
			ch5counter = 0;
			ch6counter = 0;
			ch7counter = 0;
			ch8counter = 0;
		    ch9counter = 0;
			ch10counter = 0;

  
			
			ttrigger0 = *timestamp;
			ch1counter++;
			
		}
		// below here is correct for run_4 test but not future mapping
		else if (*channel == 1 && *board == 0 && ch2counter == 0)
		{
			X1 = *timestamp;
			ch2counter++;
		}
		else if (*channel == 2 && *board == 0 && ch3counter == 0)
		{
			X2 = *timestamp;
			ch3counter++;
		}
		else if (*channel == 3 && *board == 0 && ch4counter == 0)
		{
			Y1 = *timestamp;
			ch4counter++;
		}
		else if (*channel == 4 && *board == 0 && ch5counter == 0)
		{
			Y2 = *timestamp;
			ch5counter++;
		}
		// below here is future mapping for MCP2
		else if (*channel == 5 && *board == 0 && ch6counter == 0)
		{
			T2 = *timestamp;
			ch6counter++;
		}
		else if (*channel == 6 && *board == 0 && ch7counter == 0)
		{
			X3 = *timestamp;
			ch7counter++;

		}
		else if (*channel == 7 && *board == 0 && ch8counter == 0)
		{
			X4 = *timestamp;
			ch8counter++;
		}
		else if (*channel == 0 && *board == 1 && ch9counter == 0)
		{
			Y3 = *timestamp;
			ch9counter++;
		}
		else if (*channel == 1 && *board == 1 && ch10counter == 0)
		{
			Y4 = *timestamp;
			ch10counter++;
		}
		else if (*channel == 2 && *board == 1)
		{
			h1_labr_energy->Fill(*energy);
		}
		else if (*channel == 0 && *board == 2)
		{
			h1_germanium_energy->Fill(*energy);
		}
		else if (*board == 3)
		{
			h1_dssd_energy[*channel]->Fill(*energy);
		}
		
    if (*channel == 0 && *board == 0) n_triggers++;
    if (*channel == 1 && *board == 0) n_x1++;
    if (*channel == 2 && *board == 0) n_x2++;
    if (*channel == 3 && *board == 0) n_y1++;
    if (*channel == 4 && *board == 0) n_y2++;
    if (*channel == 5 && *board == 0) n_t2++;
    if (*channel == 6 && *board == 0) n_x3++;
    if (*channel == 7 && *board == 0) n_x4++;
    if (*channel == 0 && *board == 1) n_y3++;
    if (*channel == 1 && *board == 1) n_y4++;


   // std::cout << "Board: " << *board << " - Channel: " << *channel <<  " - Time: " << *timestamp << std::endl;
		
	} // tree reader loop

	std::cout << "Complete events: " << completeEvent << "(" << 100 * completeEvent / (completeEvent + incompleteEvent) << "%)" << std::endl;
	std::cout << "Incomplete events: " << incompleteEvent<< "(" << 100 * incompleteEvent / (completeEvent + incompleteEvent) << "%)" << std::endl;

  std::cout << "Complete MCP1: " << completeMCP1 << "(" << 100 * completeMCP1/n_triggers  << "%)" << std::endl;
  std::cout << "Complete MCP2: " << completeMCP2 << std::endl;

  std::cout << "Total triggers:                 " << n_triggers << std::endl;
  std::cout << "Number of X1:                   " << n_x1 << std::endl;
  std::cout << "Number of X2:                   " << n_x2 << std::endl;
  std::cout << "Number of Y1:                   " << n_y1 << std::endl;
  std::cout << "Number of Y2:                   " << n_y2 << std::endl;
  std::cout << "Number of T2 (MPC 2)            " << n_t2 << std::endl;
  std::cout << "Number of X3:                   " << n_x3 << std::endl;
  std::cout << "Number of X4:                   " << n_x4 << std::endl;
  std::cout << "Number of Y3:                   " << n_y3 << std::endl;
  std::cout << "Number of Y4:                   " << n_y4 << std::endl;



  } // file loop
  
  std::cout << "Writing histograms to file." << std::endl;
  
  // Write histograms to file
  of->cd();
  
  c_delta_xy_MCP1->cd(1);
  h1_deltaX_mcp1->Draw();
  c_delta_xy_MCP1->cd(2);
  h1_deltaY_mcp1->Draw();
  
  c_delta_xy_MCP2->cd(1);
  h1_deltaX_mcp2->Draw();
  c_delta_xy_MCP2->cd(2);
  h1_deltaY_mcp2->Draw();
  
  c_heatmap_MCP1->cd();
  gPad->SetLogz(); // logcolorbar
  gStyle->SetOptStat(110010);
  h2_DDL_heatmap_MCP1->Draw("COLZ");
  
  c_heatmap_MCP2->cd();
  gPad->SetLogz(); // logcolorbar
  gStyle->SetOptStat(110010);
  h2_DDL_heatmap_MCP2->Draw("COLZ");
  
  // CEJ: keeping in case desired later  
  if(plotter ==1)
  {
	TAxis *xaxis = h2_DDL_heatmap_MCP1->GetXaxis();

	//et custom labels for x-axis
	for (int i = -150000; i <= 150000; i+=50000) 
	{
		TString label = TString::Format("%d", i/1000); // convert to k units
		xaxis->SetBinLabel(xaxis->FindBin(i), label);
	}

	// Get the y-axis
	TAxis *yaxis = h2_DDL_heatmap_MCP1->GetYaxis();

	// Set custom labels for y-axis
	for (int i = -150000; i <= 150000; i+=50000) 
	{
		TString label = TString::Format("%d", i/1000); // convert to k units
		yaxis->SetBinLabel(yaxis->FindBin(i), label);
	}
  }

  //TPaveStats *stats = (TPaveStats*)histogram2->FindObject("stats"); not working

  //if (stats) 
  //{
  //	stats->SetX1NDC(0.4); // Set the left edge position in NDC (normalized device coordinates)
  //	stats->SetY1NDC(0.4); // Set the bottom edge position in NDC
  //   	stats->SetX2NDC(0.4); // Set the right edge position in NDC
  //   	stats->SetY2NDC(0.4); // Set the top edge position in NDC
  //}
  ///endechatgpt
  
  of->cd();
  h1_mcp_tdiff->Write();
  h1_mcp1_event_dt->Write();
  h1_labr_energy->Write();
  h1_germanium_energy->Write();
  c_delta_xy_MCP1->Write();
  c_delta_xy_MCP2->Write();
  c_heatmap_MCP1->Write();
  c_heatmap_MCP2->Write();
  dir_dssd->cd();
  dir_dssd->Write();


  of->Close();

  // Clean up
  delete c_delta_xy_MCP1;
  delete c_delta_xy_MCP2;
  delete c_heatmap_MCP1;
  delete c_heatmap_MCP2;
  delete of;

  return 0;
}
