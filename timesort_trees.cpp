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
#include <dirent.h>
#include <unistd.h>
#include <termios.h>
#include <fnmatch.h> // Channelconfig
#include <TBranch.h>
#include <TMath.h>
#include <cstdlib>
#include <TStyle.h> // Include the header file for gStyle
#include <filesystem>

namespace fs = std::filesystem;

// :::::::::::::::::: TIME SORTER / EVENT CONSTRUCTOR :::::::::::::::::::::::::::::::::::: //
// :: This program..                                                                       //
// :: 1. Orders raw events by timestamp.                                                   //
// :: 2. Constructs MCP events.                                                            //
// :: 3. Orders MCP events by timestamp.                                                   //
// :: 4. Writes MCP event information to a tree "SortedData".                              // 
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //
// To use :: $ bash sort                                                                   //
// ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::: //

std::string Ordner = "../"; // path to run folder
std::string Runname = "run_9"; // run number
std::string dataformat = "UNFILTERED"; // only UNFILTERED seems to work/be helpful right now.. 
std::string checkfile = "Data_run*.root"; // name convention; used to check # of subruns in folder

// structure erstellen
struct Entry
{
  UShort_t Channel;
  ULong64_t Timestamp;
  UShort_t Energy;
  UShort_t Board;
};

// Store MCP information to track events
struct EntryMCP
{
    ULong64_t Trigger_Time;
    Long64_t X1;
    Long64_t X2;
    Long64_t Y1;
    Long64_t Y2;
    Bool_t Complete; // true or false
    Int_t MCP; // 1 or 2
};

// Sort by timestamp and construct event
void sortAndFillTree(const char *inputFileName, const char *outputFileName)
{
  // Open the input file and get the TTree
  TFile *inputFile = TFile::Open(inputFileName);
  TTree *inputTree = dynamic_cast<TTree *>(inputFile->Get("Data"));

  // Create a temporary array to hold the sorted entries
  std::vector<Entry> sortedEntries;

  Entry entry;

  // Set the branch addresses for reading
  inputTree->SetBranchAddress("Channel", &entry.Channel);
  inputTree->SetBranchAddress("Timestamp", &entry.Timestamp);
  inputTree->SetBranchAddress("Energy", &entry.Energy);
  inputTree->SetBranchAddress("Board", &entry.Board);

  // Loop over the entries in the TTree and fill the temporary array
  ULong64_t numEntries = inputTree->GetEntries();
  for (ULong64_t i = 0; i < numEntries; ++i)
  {
    inputTree->GetEntry(i);
  
    // timestamp shift for MPC2? doesn't seem necessary
    // if ((entry.Board == 0 && entry.Channel == 5) || (entry.Board == 0 && entry.Channel == 6) || (entry.Board == 0 && entry.Channel == 7) || (entry.Board == 1 && entry.Channel == 0) || (entry.Board == 1 && entry.Channel == 1)) 
    // {
    //   entry.Timestamp -= 90000;
    // }

    sortedEntries.push_back(entry);

  }

  // Sort raw data entries by time
  std::sort(sortedEntries.begin(), sortedEntries.end(), [](const Entry &a, const Entry &b)
            { return a.Timestamp < b.Timestamp; });

  // Clear the original TTree
  inputTree->Reset();

  std::cout << "Time sorted. Constructing events.." << std::endl;

  // keep track of event informations
  EntryMCP entryMCP1;
  entryMCP1.MCP = 1;
  EntryMCP entryMCP2;
  entryMCP2.MCP = 2;

  // counters
  int ch1counter = 0;
  int ch2counter = 0;
  int ch3counter = 0;
  int ch4counter = 0;
  int ch6counter = 0;
  int ch7counter = 0;
  int ch8counter = 0;
  int ch9counter = 0;

  std::vector<EntryMCP> all_events;

  for (auto const & entry : sortedEntries)
  {
      if (entry.Channel == 0 && entry.Board == 0)
		  {
          // Sort MCP1 events
          if (entryMCP1.Trigger_Time != 0)
          {
              // put entry in vector
              if (entryMCP1.X1 != 0 && entryMCP1.X2 != 0 && entryMCP1.Y1 != 0 && entryMCP1.Y2 != 0)
              {
                  // entry is "Complete"
                  entryMCP1.Complete = true;
              }
              else entryMCP1.Complete = false;

              all_events.emplace_back(entryMCP1);

              // Reset entry
              entryMCP1.Trigger_Time = 0;
              entryMCP1.X1 = 0;
              entryMCP1.X2 = 0;
              entryMCP1.Y1 = 0;
              entryMCP1.Y2 = 0;
              entryMCP1.Complete = false;
          }

          entryMCP1.Trigger_Time = entry.Timestamp;

          ch1counter = 0;
          ch2counter = 0;
          ch3counter = 0;
          ch4counter = 0;
      }

      if (entry.Channel == 5 && entry.Board == 0)
      {
          // Sort MCP2 events
          if (entryMCP2.Trigger_Time != 0)
          {
              // put entry in vector
              if (entryMCP2.X1 != 0 && entryMCP2.X2 != 0 && entryMCP2.Y1 != 0 && entryMCP2.Y2 != 0)
              {
                  // entry is "Complete"
                  entryMCP2.Complete = true;
              }
              else entryMCP2.Complete = false;

              all_events.emplace_back(entryMCP2);

              // Reset entry
              entryMCP2.Trigger_Time = 0;
              entryMCP2.X1 = 0;
              entryMCP2.X2 = 0;
              entryMCP2.Y1 = 0;
              entryMCP2.Y2 = 0;
              entryMCP2.Complete = false;
          }

          entryMCP2.Trigger_Time = entry.Timestamp;

          ch6counter = 0;
          ch7counter = 0;
          ch8counter = 0;
          ch9counter = 0;

      }

      if (entry.Channel == 1 && entry.Board == 0 && ch1counter == 0)
      {
          entryMCP1.X1 = entry.Timestamp;
          ch1counter++;
      }
      else if (entry.Channel == 2 && entry.Board == 0 && ch2counter == 0)
      {
          entryMCP1.X2 = entry.Timestamp;
          ch2counter++;
      }
      else if (entry.Channel == 3 && entry.Board == 0 && ch3counter == 0)
      {
          entryMCP1.Y1 = entry.Timestamp;
          ch3counter++;
      }
      else if (entry.Channel == 4 && entry.Board == 0 && ch4counter == 0)
      {
          entryMCP1.Y2 = entry.Timestamp;
          ch4counter++;
      }
      else if (entry.Channel == 6 && entry.Board == 0 && ch6counter == 0)
      {
          entryMCP2.X1 = entry.Timestamp;
          ch6counter++;
      }
      else if (entry.Channel == 7 && entry.Board == 0 && ch7counter == 0)
      {
          entryMCP2.X2 = entry.Timestamp;
          ch7counter++;
      }
      else if (entry.Channel == 0 && entry.Board == 1 && ch8counter == 0)
      {
          entryMCP2.Y1 = entry.Timestamp;
          ch8counter++;
      }
      else if (entry.Channel == 1 && entry.Board == 1 && ch9counter == 0)
      {
          entryMCP2.Y2 = entry.Timestamp;
          ch9counter++;
      }

  } // sorted entries loop


  std::cout << "Events constructed. Correcting time order.." << std::endl;
  std::sort(all_events.begin(), all_events.end(), [](const EntryMCP &a, const EntryMCP &b)
            { return a.Trigger_Time < b.Trigger_Time; });

  std::cout << "Time ordering correct. Writing Tree.." << std::endl;

  // Create a new output file and TTree
  TFile outputFile(outputFileName, "RECREATE");
  TTree *outputTree = new TTree("SortedData", "Sorted Tree");

  // Create variables to hold the sorted entries
  ULong64_t Trigger_Time;
  Long64_t X1;
  Long64_t X2;
  Long64_t Y1;
  Long64_t Y2;
  Bool_t Complete;
  Int_t MCP;
   
  // Set the branch addresses for writing
  outputTree->Branch("Trigger_Time", &Trigger_Time);
  outputTree->Branch("X1", &X1);
  outputTree->Branch("X2", &X2);
  outputTree->Branch("Y1", &Y1);
  outputTree->Branch("Y2", &Y2);
  outputTree->Branch("Complete", &Complete);
  outputTree->Branch("MCP", &MCP);

  // Loop over the sorted entries array and fill the new TTree
  for (const auto &event : all_events)
  {
      Trigger_Time = event.Trigger_Time;
      X1 = event.X1;
      X2 = event.X2;
      Y1 = event.Y1;
      Y2 = event.Y2;
      Complete = event.Complete;
      MCP = event.MCP;
      outputTree->Fill();
  }

  outputTree->Write();

  outputFile.Close();
  inputFile->Close();
}

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
  if (!fs::exists(SfolderPath)) fs::create_directories(SfolderPath);

  std::string baseFileName = folderPath + "/Data_" + Runname;
  std::string SfileName = SfolderPath + "/Data_" + Runname;

  std::cout << ": " << folderPath << std::endl;

  DIR *dir;
  struct dirent *entry;


  int maxFileCount = 0;
  int dataPointsCounter = 0;
  int fileCount = 0;
  bool foundMatchingFile = false;
  int reruns = 0;

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
    std::stringstream filePathStream;
    std::stringstream SfilePathStream;
    filePathStream << baseFileName;
    SfilePathStream << SfileName;
    if (i > 0)
    { // kondition das der erste file richtige gelesen wird
      filePathStream << "_" << i;
      SfilePathStream << "_" << i;
    }
    std::string filePath = filePathStream.str() + ".root";
    std::string SfilePath = SfilePathStream.str() + "_timesorted.root";

    const char *filepathCStr = filePath.c_str();
    const char *SfilepathCStr = SfilePath.c_str();

    std::cout << "start sorting: " << baseFileName << i << std::endl;

    sortAndFillTree(filepathCStr, SfilepathCStr);
    std::cout << "completed" << std::endl;
    
  } // Loop files

  return 0;
}
