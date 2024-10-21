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
#include <TBranch.h>
#include <TMath.h>
#include <cstdlib>
#include <TStyle.h> // Include the header file for gStyle
#include <filesystem>

namespace fs = std::filesystem;


std::string Ordner = "./";
std::string Runname = "run_9";        // no
std::string dataformat = "UNFILTERED"; // no/
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
// timestamp sorting
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
  
    // should apply proper mapping function at some point
    // if ((entry.Board == 0 && entry.Channel == 5) || (entry.Board == 0 && entry.Channel == 6) || (entry.Board == 0 && entry.Channel == 7) || (entry.Board == 1 && entry.Channel == 0) || (entry.Board == 1 && entry.Channel == 1)) 
    // {
    //   entry.Timestamp -= 90000;
    // }

    sortedEntries.push_back(entry);

  }

  // Sort the entries based on the timestamps in ascending order
  // std::cerr << "starrrsorting." << std::endl;

  std::sort(sortedEntries.begin(), sortedEntries.end(), [](const Entry &a, const Entry &b)
            { return a.Timestamp < b.Timestamp; });
  // std::cerr << "starr2rsorting." << std::endl;

  // Clear the original TTree
  inputTree->Reset();

  // Create a new output file and TTree
  TFile outputFile(outputFileName, "recreate");
  TTree *outputTree = new TTree("SortedData", "Sorted Tree");

  // Create variables to hold the sorted entries
  UShort_t sortedChannel;
  ULong64_t sortedTimestamp;
  UShort_t sortedEnergy;
  UShort_t sortedBoard;

  // Set the branch addresses for writing
  outputTree->Branch("Channel", &sortedChannel);
  outputTree->Branch("Timestamp", &sortedTimestamp);
  outputTree->Branch("Energy", &sortedEnergy);
  outputTree->Branch("Board", &sortedBoard);
  // Loop over the sorted entries array and fill the new TTree
  for (const auto &sortedEntry : sortedEntries)
  {
    sortedChannel = sortedEntry.Channel;
    sortedTimestamp = sortedEntry.Timestamp;
    sortedEnergy = sortedEntry.Energy;
    sortedBoard = sortedEntry.Board;
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

  std::string baseFileName = Ordner + Runname + "/" + dataformat + "/Data_" + Runname;
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
    
    // CEJ: finish first phase here, only need to timesort once
  } // loop files

  return 0;
}
