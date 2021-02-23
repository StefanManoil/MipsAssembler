#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <stdexcept>
#include "scanner.h"

class Node {
  Token::Kind type;
  //std::string lexeme;
  bool finishingNode;
  //int valueIfNecessary; // get this from the lexeme
  std::map<Token::Kind, Node*> nextPossibleNodes;
  public:
      Node() {
      }
      Node(Token::Kind type, bool finishingNode, const std::map<Token::Kind, Node*>& nextPossibleNodes) {
        this->type = type;
        //this->lexeme = lexeme;
        this->finishingNode = finishingNode;
        this->nextPossibleNodes = nextPossibleNodes;
      }
      Node(Token::Kind type, bool finishingNode, std::map<Token::Kind, Node*>&& nextPossibleNodes) {
        this->type = type;
        this->finishingNode = finishingNode;
        this->nextPossibleNodes = std::move(nextPossibleNodes);
      }
      // VV do not need this copy constructor anymore, I decided that corresponding instructions with and without labels should share the appropriate linked list
      // because if not we would have double the linked lists and this is not really necessary
      /*Node(const Node &other) {
        this->type = other.type;
        this->finishingNode = other.finishingNode;
        //this->nextPossibleNodes = other.nextPossibleNodes;
        std::map<Token::Kind, Node*> newNodes;
        for (auto& keyValuePair : other.nextPossibleNodes) {
          newNodes.emplace(keyValuePair.first, new Node(keyValuePair.second));
        }
        this->nextPossibleNodes = newNodes;
      }*/
      ~Node() {
        for (auto& keyValuePair : nextPossibleNodes) {
          if (keyValuePair.second != this) {
            delete keyValuePair.second;
            keyValuePair.second = nullptr;
          }
          else {
            keyValuePair.second = nullptr;
          }
        }
      }
      Token::Kind getType() {
        return type;
      }
      std::string getTypeAsString() {
        std::string typeString;
        switch (type) {
          case Token::ID:         typeString = "ID";         break;
          case Token::LABEL:      typeString = "LABEL";      break;
          case Token::WORD:       typeString = "WORD";       break;
          case Token::COMMA:      typeString = "COMMA";      break;
          case Token::LPAREN:     typeString = "LPAREN";     break;
          case Token::RPAREN:     typeString = "RPAREN";     break;
          case Token::INT:        typeString = "INT";        break;
          case Token::HEXINT:     typeString = "HEXINT";     break;
          case Token::REG:        typeString = "REG";        break;
          case Token::WHITESPACE: typeString = "WHITESPACE"; break;
          case Token::COMMENT:    typeString = "COMMENT";    break;
        }
        return typeString;
      }
      /*std::string getLexeme() {
        return lexeme;
      }*/
      bool isFinishingNode() {
        return finishingNode;
      }
      /*int getValue() {
        return valueIfNecessary;
      }*/
      const std::map<Token::Kind, Node*>& getNextPossibleNodes() {
        return nextPossibleNodes;
      }
      void addNode(std::pair<Token::Kind, Node*>& newNode) {
        nextPossibleNodes.emplace(newNode);
      }
      void clearNextNodes() {
        for (auto& pair: nextPossibleNodes) {
          pair.second = nullptr;
        }
        nextPossibleNodes.clear();
      }
};

/*
 * C++ Starter code for CS241 A3
 * All code requires C++14, so if you're getting compile errors make sure to
 * use -std=c++14.
 *
 * This file contains the main function of your program. By default, it just
 * prints the scanned list of tokens back to standard output.
 */
int main() {
  std::string line;

  // the symbol table for keeping track of defined labels
  std::map<std::string, int> symbolTable;

  // location tracker for labels on lines
  int locationCounter = 0;

  std::vector<std::vector<Token>> intermediateRepresentations; // a vector of intermediate representations we will parse through in the Synthesis stage

  // let us create all the possible LinkedLists of valid instructions
  // this will be a map of strings to Node pointers
  std::map<std::string, Node*> instructionLists;

  // these will be used only for deallocation purposes
  std::vector<Node*> instructionsWithLabels; // we will iterate through these nodes, delete them and set them all to nullptr
  std::vector<Node*> instructionsNoLabels; // we will only set these to nullptr as they have already been deallocated

  // will build the nodes in reverse order for simplicity --------------------------------------------------------------------------------------------------------
  // also instead of creating really similar list nodes twice without the initial Label node, create pointers to the second node of each instruction
  // so that both label-preceded instructions and non-label preceded instructions can share the same LinkedList for parsing

  // components for .word with label(s) instruction -----------------------------------------
  Node* hexIntWordWithLabel = new Node(Token::HEXINT, true, {});
  Node* intWordWithLabel = new Node(Token::INT, true, {});
  Node* idWordWithLabel = new Node(Token::ID, true, {}); // actually a label but perceived as id by token

  std::map<Token::Kind, Node*> mapForInstructionNameWordWithLabel;
  mapForInstructionNameWordWithLabel.emplace(Token::HEXINT, hexIntWordWithLabel);
  mapForInstructionNameWordWithLabel.emplace(Token::INT, intWordWithLabel);
  mapForInstructionNameWordWithLabel.emplace(Token::ID, idWordWithLabel); // actually a label but perceived as id by token
  Node* instructionNameWordWithLabel = new Node(Token::WORD, false, mapForInstructionNameWordWithLabel);

  std::map<Token::Kind, Node*> mapForLabelWordWithLabel;
  mapForLabelWordWithLabel.emplace(Token::WORD, instructionNameWordWithLabel);
  Node* instructionLabelWordWithLabel = new Node(Token::LABEL, false, mapForLabelWordWithLabel);
  std::pair<Token::Kind, Node*> instructionLabelWordWithLabelPointToItself(Token::LABEL, instructionLabelWordWithLabel);
  instructionLabelWordWithLabel->addNode(instructionLabelWordWithLabelPointToItself);

  instructionLists.emplace("wordWithLabel", instructionLabelWordWithLabel);
  instructionsWithLabels.emplace_back(instructionLabelWordWithLabel);
  // end components for .word with label(s) instruction -----------------------------------------
  // components for .word without label(s) instruction -----------------------------------------
  Node* instructionWordWithoutLabel = instructionNameWordWithLabel;
  instructionLists.emplace("wordNoLabel", instructionWordWithoutLabel);
  instructionsNoLabels.emplace_back(instructionWordWithoutLabel);
  // end components for .word without label(s) instruction -----------------------------------------

  // components for beq, bne with label(s) instruction -----------------------------------------
  Node* hexIntBeqBneWithLabel = new Node(Token::HEXINT, true, {});
  Node* intBeqBneWithLabel = new Node(Token::INT, true, {});
  Node* idBeqBneWithLabel = new Node(Token::ID, true, {}); // actually a label but perceived as id by token

  std::map<Token::Kind, Node*> mapForSecondCommaBeqBneWithLabel;
  mapForSecondCommaBeqBneWithLabel.emplace(Token::HEXINT, hexIntBeqBneWithLabel);
  mapForSecondCommaBeqBneWithLabel.emplace(Token::INT, intBeqBneWithLabel);
  mapForSecondCommaBeqBneWithLabel.emplace(Token::ID, idBeqBneWithLabel);
  Node* secondCommaBeqBneWithLabel = new Node(Token::COMMA, false, mapForSecondCommaBeqBneWithLabel);

  std::map<Token::Kind, Node*> mapForSecondRegisterBeqBneWithLabel;
  mapForSecondRegisterBeqBneWithLabel.emplace(Token::COMMA, secondCommaBeqBneWithLabel);
  Node* secondRegisterBeqBneWithLabel = new Node(Token::REG, false, mapForSecondRegisterBeqBneWithLabel);

  std::map<Token::Kind, Node*> mapForFirstCommaBeqBneWithLabel;
  mapForFirstCommaBeqBneWithLabel.emplace(Token::REG, secondRegisterBeqBneWithLabel);
  Node* firstCommaBeqBneWithLabel = new Node(Token::COMMA, false, mapForFirstCommaBeqBneWithLabel);

  std::map<Token::Kind, Node*> mapForFirstRegisterBeqBneWithLabel;
  mapForFirstRegisterBeqBneWithLabel.emplace(Token::COMMA, firstCommaBeqBneWithLabel);
  Node* firstRegisterBeqBneWithLabel = new Node(Token::REG, false, mapForFirstRegisterBeqBneWithLabel);

  std::map<Token::Kind, Node*> mapForInstructionNameBeqBneWithLabel;
  mapForInstructionNameBeqBneWithLabel.emplace(Token::REG, firstRegisterBeqBneWithLabel);
  Node* instructionNameBeqBneWithLabel = new Node(Token::ID, false, mapForInstructionNameBeqBneWithLabel);

  std::map<Token::Kind, Node*> mapForLabelBeqBneWithLabel;
  mapForLabelBeqBneWithLabel.emplace(Token::ID, instructionNameBeqBneWithLabel);
  Node* instructionLabelBeqBneWithLabel = new Node(Token::LABEL, false, mapForLabelBeqBneWithLabel);
  std::pair<Token::Kind, Node*> instructionLabelBeqBneWithLabelPointToItself(Token::LABEL, instructionLabelBeqBneWithLabel);
  instructionLabelBeqBneWithLabel->addNode(instructionLabelBeqBneWithLabelPointToItself);

  instructionLists.emplace("branchWithLabel", instructionLabelBeqBneWithLabel);
  instructionsWithLabels.emplace_back(instructionLabelBeqBneWithLabel);
  // end components for beq, bne with label(s) instruction -----------------------------------------
  // components for beq, bne without label(s) instruction -----------------------------------------
  Node* instructionBeqBneWithoutLabel = instructionNameBeqBneWithLabel;
  instructionLists.emplace("branchNoLabel", instructionBeqBneWithoutLabel);
  instructionsNoLabels.emplace_back(instructionBeqBneWithoutLabel);
  // end components for beq, bne without label(s) instruction -----------------------------------------

  // components for add, sub, slt, sltu with label(s) instruction -----------------------------------------
  Node* thirdRegisterAddSubSltSltuWithLabel = new Node(Token::REG, true, {});

  std::map<Token::Kind, Node*> mapForSecondCommaAddSubSltSltuWithLabel;
  mapForSecondCommaAddSubSltSltuWithLabel.emplace(Token::REG, thirdRegisterAddSubSltSltuWithLabel);
  Node* secondCommaAddSubSltSltuWithLabel = new Node(Token::COMMA, false, mapForSecondCommaAddSubSltSltuWithLabel);

  std::map<Token::Kind, Node*> mapForSecondRegisterAddSubSltSltuWithLabel;
  mapForSecondRegisterAddSubSltSltuWithLabel.emplace(Token::COMMA, secondCommaAddSubSltSltuWithLabel);
  Node* secondRegisterAddSubSltSltuWithLabel = new Node(Token::REG, false, mapForSecondRegisterAddSubSltSltuWithLabel);

  std::map<Token::Kind, Node*> mapForFirstCommaAddSubSltSltuWithLabel;
  mapForFirstCommaAddSubSltSltuWithLabel.emplace(Token::REG, secondRegisterAddSubSltSltuWithLabel);
  Node* firstCommaAddSubSltSltuWithLabel = new Node(Token::COMMA, false, mapForFirstCommaAddSubSltSltuWithLabel);

  std::map<Token::Kind, Node*> mapForFirstRegisterAddSubSltSltuWithLabel;
  mapForFirstRegisterAddSubSltSltuWithLabel.emplace(Token::COMMA, firstCommaAddSubSltSltuWithLabel);
  Node* firstRegisterAddSubSltSltuWithLabel = new Node(Token::REG, false, mapForFirstRegisterAddSubSltSltuWithLabel);

  std::map<Token::Kind, Node*> mapForInstructionNameAddSubSltSltuWithLabel;
  mapForInstructionNameAddSubSltSltuWithLabel.emplace(Token::REG, firstRegisterAddSubSltSltuWithLabel);
  Node* instructionNameAddSubSltSltuWithLabel = new Node(Token::ID, false, mapForInstructionNameAddSubSltSltuWithLabel);

  std::map<Token::Kind, Node*> mapForLabelAddSubSltSltuWithLabel;
  mapForLabelAddSubSltSltuWithLabel.emplace(Token::ID, instructionNameAddSubSltSltuWithLabel);
  Node* instructionLabelAddSubSltSltuWithLabel = new Node(Token::LABEL, false, mapForLabelAddSubSltSltuWithLabel);
  std::pair<Token::Kind, Node*> instructionLabelAddSubSltSltuWithLabelPointToItself(Token::LABEL, instructionLabelAddSubSltSltuWithLabel);
  instructionLabelAddSubSltSltuWithLabel->addNode(instructionLabelAddSubSltSltuWithLabelPointToItself);

  instructionLists.emplace("addSubSltSltuWithLabel", instructionLabelAddSubSltSltuWithLabel);
  instructionsWithLabels.emplace_back(instructionLabelAddSubSltSltuWithLabel);
  // end components for add, sub, slt, sltu with label(s) instruction -----------------------------------------
  // components for add, sub, slt, sltu without label(s) instruction -----------------------------------------
  Node* instructionAddSubSltSltuWithoutLabel = instructionNameAddSubSltSltuWithLabel;
  instructionLists.emplace("addSubSltSltuNoLabel", instructionAddSubSltSltuWithoutLabel);
  instructionsNoLabels.emplace_back(instructionAddSubSltSltuWithoutLabel);
  // end components for add, sub, slt, sltu without label(s) instruction -----------------------------------------

  // components for mult, multu, div, divu with label(s) instruction -----------------------------------------
  Node* secondRegisterMultMultuDivDivuWithLabel = new Node(Token::REG, true, {});

  std::map<Token::Kind, Node*> mapForCommaMultMultuDivDivuWithLabel;
  mapForCommaMultMultuDivDivuWithLabel.emplace(Token::REG, secondRegisterMultMultuDivDivuWithLabel);
  Node* commaMultMultuDivDivuWithLabel = new Node(Token::COMMA, false, mapForCommaMultMultuDivDivuWithLabel);

  std::map<Token::Kind, Node*> mapForFirstRegisterMultMultuDivDivuWithLabel;
  mapForFirstRegisterMultMultuDivDivuWithLabel.emplace(Token::COMMA, commaMultMultuDivDivuWithLabel);
  Node* firstRegisterMultMultuDivDivuWithLabel = new Node(Token::REG, false, mapForFirstRegisterMultMultuDivDivuWithLabel);

  std::map<Token::Kind, Node*> mapForInstructionNameMultMultuDivDivuWithLabel;
  mapForInstructionNameMultMultuDivDivuWithLabel.emplace(Token::REG, firstRegisterMultMultuDivDivuWithLabel);
  Node* instructionNameMultMultuDivDivuWithLabel = new Node(Token::ID, false, mapForInstructionNameMultMultuDivDivuWithLabel);

  std::map<Token::Kind, Node*> mapForLabelMultMultuDivDivuWithLabel;
  mapForLabelMultMultuDivDivuWithLabel.emplace(Token::ID, instructionNameMultMultuDivDivuWithLabel);
  Node* instructionLabelMultMultuDivDivuWithLabel = new Node(Token::LABEL, false, mapForLabelMultMultuDivDivuWithLabel);
  std::pair<Token::Kind, Node*> instructionLabelMultMultuDivDivuWithLabelPointToItself(Token::LABEL, instructionLabelMultMultuDivDivuWithLabel);
  instructionLabelMultMultuDivDivuWithLabel->addNode(instructionLabelMultMultuDivDivuWithLabelPointToItself);

  instructionLists.emplace("multMultuDivDivuWithLabel", instructionLabelMultMultuDivDivuWithLabel);
  instructionsWithLabels.emplace_back(instructionLabelMultMultuDivDivuWithLabel);
  // end components for mult, multu, div, divu with label(s) instruction -----------------------------------------
  // components for mult, multu, div, divu without label(s) instruction -----------------------------------------
  Node* instructionMultMultuDivDivuWithoutLabel = instructionNameMultMultuDivDivuWithLabel;
  instructionLists.emplace("multMultuDivDivuNoLabel", instructionMultMultuDivDivuWithoutLabel);
  instructionsNoLabels.emplace_back(instructionMultMultuDivDivuWithoutLabel);
  // end components for mult, multu, div, divu without label(s) instruction -----------------------------------------

  // components for mfhi, mflo, lis, jr, jalr with label(s) instruction -----------------------------------------
  Node* registerMfhiMfloLisJrJalrWithLabel = new Node(Token::REG, true, {});

  std::map<Token::Kind, Node*> mapForInstructionNameMfhiMfloLisJrJalrWithLabel;
  mapForInstructionNameMfhiMfloLisJrJalrWithLabel.emplace(Token::REG, registerMfhiMfloLisJrJalrWithLabel);
  Node* instructionNameMfhiMfloLisJrJalrWithLabel = new Node(Token::ID, false, mapForInstructionNameMfhiMfloLisJrJalrWithLabel);

  std::map<Token::Kind, Node*> mapForLabelMfhiMfloLisJrJalrWithLabel;
  mapForLabelMfhiMfloLisJrJalrWithLabel.emplace(Token::ID, instructionNameMfhiMfloLisJrJalrWithLabel);
  Node* instructionLabelMfhiMfloLisJrJalrWithLabel = new Node(Token::LABEL, false, mapForLabelMfhiMfloLisJrJalrWithLabel);
  std::pair<Token::Kind, Node*> instructionLabelMfhiMfloLisJrJalrWithLabelPointToItself(Token::LABEL, instructionLabelMfhiMfloLisJrJalrWithLabel);
  instructionLabelMfhiMfloLisJrJalrWithLabel->addNode(instructionLabelMfhiMfloLisJrJalrWithLabelPointToItself);

  instructionLists.emplace("mfhiMfloLisJrJalrWithLabel", instructionLabelMfhiMfloLisJrJalrWithLabel);
  instructionsWithLabels.emplace_back(instructionLabelMfhiMfloLisJrJalrWithLabel);
  // end components for mfhi, mflo, lis, jr, jalr with label(s) instruction -----------------------------------------
  // components for mfhi, mflo, lis, jr, jalr without label(s) instruction -----------------------------------------
  Node* instructionMfhiMfloLisJrJalrWithoutLabel = instructionNameMfhiMfloLisJrJalrWithLabel;
  instructionLists.emplace("mfhiMfloLisJrJalrNoLabel", instructionMfhiMfloLisJrJalrWithoutLabel);
  instructionsNoLabels.emplace_back(instructionMfhiMfloLisJrJalrWithoutLabel);
  // end components for mfhi, mflo, lis, jr, jalr without label(s) instruction -----------------------------------------

  // add components for lw, sw with label(s) instruction -----------------------------------------
  Node* rparenLwSwWithLabel = new Node(Token::RPAREN, true, {});

  std::map<Token::Kind, Node*> mapForRegisterInParenLwSwWithLabel;
  mapForRegisterInParenLwSwWithLabel.emplace(Token::RPAREN, rparenLwSwWithLabel);
  Node* registerInParenLwSwWithLabel = new Node(Token::REG, false, mapForRegisterInParenLwSwWithLabel);

  std::map<Token::Kind, Node*> mapForLparenLwSwWithLabel;
  mapForLparenLwSwWithLabel.emplace(Token::REG, registerInParenLwSwWithLabel);
  Node* lparenLwSwWithLabel = new Node(Token::LPAREN, false, mapForLparenLwSwWithLabel);

  std::map<Token::Kind, Node*> mapForHexIntLwSwWithLabel;
  mapForHexIntLwSwWithLabel.emplace(Token::LPAREN, lparenLwSwWithLabel);
  Node* hexIntLwSwWithLabel = new Node(Token::HEXINT, false, mapForHexIntLwSwWithLabel);
  std::map<Token::Kind, Node*> mapForIntLwSwWithLabel;
  mapForIntLwSwWithLabel.emplace(Token::LPAREN, lparenLwSwWithLabel);
  Node* intLwSwWithLabel = new Node(Token::INT, false, mapForIntLwSwWithLabel);
  // if List is deleted from the head pointer as normal using destructor recursion this will be an issue at this point:
  //              HEXINT VV
  // ....  COMMA ^^ vv    LPAREN -> ...
  //                 INT ^^
  // If list is deleted normally one of the paths will be fine, but the other will be then pointing to memory that was already deallocated and this is a problem
  // maybe a good solution would be when, deleting lw/sw instruction List, delete everything from LPAREN onwards first, then set the pointers
  // in HEXINT and INT that point to LPAREN to nullptr and also empty their respective maps, and then it is safe to call delete on start of the instruction List

  std::map<Token::Kind, Node*> mapForCommaLwSwWithLabel;
  mapForCommaLwSwWithLabel.emplace(Token::HEXINT, hexIntLwSwWithLabel);
  mapForCommaLwSwWithLabel.emplace(Token::INT, intLwSwWithLabel);
  Node* commaLwSwWithLabel = new Node(Token::COMMA, false, mapForCommaLwSwWithLabel);

  std::map<Token::Kind, Node*> mapForFirstRegisterLwSwWithLabel;
  mapForFirstRegisterLwSwWithLabel.emplace(Token::COMMA, commaLwSwWithLabel);
  Node* firstRegisterLwSwWithLabel = new Node(Token::REG, false, mapForFirstRegisterLwSwWithLabel);

  std::map<Token::Kind, Node*> mapForInstructionNameLwSwWithLabel;
  mapForInstructionNameLwSwWithLabel.emplace(Token::REG, firstRegisterLwSwWithLabel);
  Node* instructionNameLwSwWithLabel = new Node(Token::ID, false, mapForInstructionNameLwSwWithLabel);

  std::map<Token::Kind, Node*> mapForLabelLwSwWithLabel;
  mapForLabelLwSwWithLabel.emplace(Token::ID, instructionNameLwSwWithLabel);
  Node* instructionLabelLwSwWithLabel = new Node(Token::LABEL, false, mapForLabelLwSwWithLabel);
  std::pair<Token::Kind, Node*> instructionLabelLwSwWithLabelPointToItself(Token::LABEL, instructionLabelLwSwWithLabel);
  instructionLabelLwSwWithLabel->addNode(instructionLabelLwSwWithLabelPointToItself);

  instructionLists.emplace("lwSWWithLabel", instructionLabelLwSwWithLabel);
  instructionsWithLabels.emplace_back(instructionLabelLwSwWithLabel);
  // end add components for lw, sw with label(s) instruction -----------------------------------------
  // components for lw, sw without label(s) instruction -----------------------------------------
  Node* instructionLwSwWithoutLabel = instructionNameLwSwWithLabel;
  instructionLists.emplace("lwSWNoLabel", instructionLwSwWithoutLabel);
  instructionsNoLabels.emplace_back(instructionLwSwWithoutLabel);
  // end components for lw, sw without label(s) instruction -----------------------------------------

  // components for label(s) only -----------------------------------------
  //std::map<Token::Kind, Node*> mapForLabelOnly;
  Node* labelOnly = new Node(Token::LABEL, true, {});
  std::pair<Token::Kind, Node*> labelOnlyPointToItself(Token::LABEL, labelOnly);
  labelOnly->addNode(labelOnlyPointToItself);

  instructionLists.emplace("labelOnly", labelOnly);
  instructionsWithLabels.emplace_back(labelOnly);
  // end components for label(s) only -----------------------------------------

  // done building LinkedLists  --------------------------------------------------------------------------------------------------------

  try {
    while (getline(std::cin, line)) {
      // This example code just prints the scanned tokens on each line.

      //std::vector<Token> tokenLine = scan(line);

      std::pair<std::vector<Token>, std::set<std::string>> tokensAndInstructionTypes = scan(line);
      std::vector<Token> tokens = tokensAndInstructionTypes.first;
      std::set<std::string> instructionTypes = tokensAndInstructionTypes.second;

      // if we scanned the line and its only WHITESPACE or a line with a COMMENT on it we can skip this line
      // as we do not want to assign a location to it; a line number
      if (tokens.size() == 0) {
        continue;
      }

      // storing all the lexemes of the current line in this string, its only use is for descriptive error message in try-catch when throwing runtime errors
      /*std::string currentLineText = "";
      for (int i = 0; i < tokens.size(); i++) {
        currentLineText += tokens.at(i).getLexeme() + " ";
      }*/


      std::string instructionType; // type of instruction we are trying to parse through
      Node* currentNode; // node we will use to parse through a particular list for analysis

      std::vector<Token> intermediateRepresentation; // an intermediate representation of the current scanned Token sequence that will be passed on to Syntheis stage

      // now let's see based on what we have in the instructionTypes set i.e. the operand format, which type of instruction list we should try to parse through ---------
      if (instructionTypes.count("label") > 0) {
        if (instructionTypes.count("word") > 0) {
          instructionType = "wordWithLabel";
          currentNode = instructionLists.at("wordWithLabel");
        }
        else {
          if (instructionTypes.count("beq") > 0 || instructionTypes.count("bne") > 0) {
            instructionType = "branchWithLabel";
            currentNode = instructionLists.at("branchWithLabel");
          }
          else if (instructionTypes.count("add") > 0 || instructionTypes.count("sub") > 0 || instructionTypes.count("slt") > 0 || instructionTypes.count("sltu") > 0) {
            instructionType = "addSubSltSltuWithLabel";
            currentNode = instructionLists.at("addSubSltSltuWithLabel");
          }
          else if (instructionTypes.count("mult") > 0 || instructionTypes.count("multu") > 0 || instructionTypes.count("div") > 0 || instructionTypes.count("divu") > 0) {
            instructionType = "multMultuDivDivuWithLabel";
            currentNode = instructionLists.at("multMultuDivDivuWithLabel");
          }
          else if (instructionTypes.count("mfhi") > 0 || instructionTypes.count("mflo") > 0 || instructionTypes.count("lis") > 0 || instructionTypes.count("jr") > 0
                  || instructionTypes.count("jalr") > 0) {
            instructionType = "mfhiMfloLisJrJalrWithLabel";
            currentNode = instructionLists.at("mfhiMfloLisJrJalrWithLabel");
          }
          else if (instructionTypes.count("lw") > 0 || instructionTypes.count("sw") > 0) {
            instructionType = "lwSWWithLabel";
            currentNode = instructionLists.at("lwSWWithLabel");
          }
          else {
            // only labels on the line
            //std::cout << "currentNode points to labelOnly list" << std::endl;
            instructionType = "labelOnly";
            currentNode = instructionLists.at("labelOnly");
          }
        }
      }
      else {
        if (instructionTypes.count("word") > 0) {
          instructionType = "wordNoLabel";
          currentNode = instructionLists.at("wordNoLabel");
        }
        else {
          if (instructionTypes.count("beq") > 0 || instructionTypes.count("bne") > 0) {
            instructionType = "branchNoLabel";
            currentNode = instructionLists.at("branchNoLabel");
          }
          else if (instructionTypes.count("add") > 0 || instructionTypes.count("sub") > 0 || instructionTypes.count("slt") > 0 || instructionTypes.count("sltu") > 0) {
            instructionType = "addSubSltSltuNoLabel";
            currentNode = instructionLists.at("addSubSltSltuNoLabel");
          }
          else if (instructionTypes.count("mult") > 0 || instructionTypes.count("multu") > 0 || instructionTypes.count("div") > 0 || instructionTypes.count("divu") > 0) {
            instructionType = "multMultuDivDivuNoLabel";
            currentNode = instructionLists.at("multMultuDivDivuNoLabel");
          }
          else if (instructionTypes.count("mfhi") > 0 || instructionTypes.count("mflo") > 0 || instructionTypes.count("lis") > 0 || instructionTypes.count("jr") > 0
                  || instructionTypes.count("jalr") > 0) {
            instructionType = "mfhiMfloLisJrJalrNoLabel";
            currentNode = instructionLists.at("mfhiMfloLisJrJalrNoLabel");
          }
          else if (instructionTypes.count("lw") > 0 || instructionTypes.count("sw") > 0) {
            instructionType = "lwSWNoLabel";
            currentNode = instructionLists.at("lwSWNoLabel");
          }
          else {
            currentNode = nullptr;
            /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
            "\nThis is not a valid instruction");*/
            throw std::runtime_error("ERROR");
            //throw "ERROR, this does not seem to match any type of valid instruction!";
          }
        }
      }
      // -------------------------------------------------------------------------------------------------------

      // now let us do the first pass for the Analysis, this is where we will verify that labels are unique, verify tokens match intented instruction format,
      // verify registers are in range if they are present, verify ints are in range if present, verify hexints are in range if present
      // we will also build up the intermediate representation of this current line for the Synthesis stage
      for (int i = 0; i < tokens.size(); i++) {
        Token currentToken = tokens.at(i);
        Token::Kind currentTokenKind = tokens.at(i).getKind();
        std::string currentTokenLexeme = tokens.at(i).getLexeme();
        if (i != 0) {
          if (currentNode->getNextPossibleNodes().count(currentTokenKind) > 0) {
            currentNode = currentNode->getNextPossibleNodes().at(currentTokenKind);
          }
          else {
            //std::string currentNodeKindAsString = currentNode->getTypeAsString();
            // I thought about adding more descriptive error here but its a hassle to get a string version of Token::Kind from Node maps
            /*std::string expectedThings = "";
            for (auto& pair : currentNode->getNextPossibleNodes()) {
              expectedThings += pair.first.g
            }*/
            currentNode = nullptr;
            /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
            "\nExpected something else but got " + currentToken.getKindAsString() + " {" + currentToken.getLexeme() + "}");*/
            throw std::runtime_error("ERROR");
            //throw "ERROR, this is an incorrect way to produce this instruction: " + instructionType + "!";
          }
        }
        if (currentNode->getType() != currentTokenKind) {
          currentNode = nullptr;
          /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
          "\nExpecting opcode, label, or directive, but got " + currentToken.getKindAsString() + " {" + currentTokenLexeme + "}"); */
          throw std::runtime_error("ERROR");
          //throw "ERROR, this is an incorrect way to produce this instruction: " + instructionType + "!";
        }
        else if (currentTokenKind == Token::LABEL) {
          if (symbolTable.count(currentTokenLexeme) > 0) {
            currentNode = nullptr;
            /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
            "\nDuplicate symbol:  " + currentTokenLexeme);*/
            throw std::runtime_error("ERROR");
            //throw "ERROR, duplicate labels found, remember labels must be unique!";
          }
          else {
            //std::cout << currentTokenLexeme.substr(0, currentTokenLexeme.length() - 1) << " " << locationCounter << std::endl;
            //std::cerr << currentTokenLexeme.substr(0, currentTokenLexeme.length() - 1) << " " << locationCounter << std::endl;
            symbolTable.emplace(currentTokenLexeme, locationCounter);
          }
        }
        else if (currentTokenKind == Token::REG) {
          int64_t registerValue = currentToken.toNumber();
          if (registerValue < 0 || registerValue > 31) {
            currentNode = nullptr;
            /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
            "\nConstant out of range:  " + currentTokenLexeme);*/
            throw std::runtime_error("ERROR");
            //throw "ERROR, you can only use register $0 - $31!";
          }
          intermediateRepresentation.emplace_back(currentToken);
        }
        else if (currentTokenKind == Token::INT) {
          int64_t intValue = currentToken.toNumber();
          // must do different things depending if it is a .word instruction or beq/bne // lw, sw instruction
          if (instructionType == "wordWithLabel" || instructionType == "wordNoLabel") {
            if (intValue > 4294967295 || intValue < -2147483648) {
              currentNode = nullptr;
              /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
              "\nConstant out of range: " + currentTokenLexeme);*/
              throw std::runtime_error("ERROR");
              //throw "ERROR, the immediate decimal value for this .word instruction is not in range!";
            }
          }
          else {
            if (intValue > 32767 || intValue < -32768) {
              currentNode = nullptr;
              /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
              "\nBranch offset out of range");*/
              throw std::runtime_error("ERROR");
              //throw "ERROR, the immediate decimal value for this instruction is not in range!";
            }
          }
          intermediateRepresentation.emplace_back(currentToken);
        }
        else if (currentTokenKind == Token::HEXINT) {
          int64_t hexValue = currentToken.toNumber();
          // must do different things depending if it is a .word instruction or beq/bne // lw, sw instruction
          if (instructionType == "wordWithLabel" || instructionType == "wordNoLabel") {
            if (hexValue < 0 || hexValue > 4294967295) {
              currentNode = nullptr;
              /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
              "\nConstant out of range: " + currentTokenLexeme);*/
              throw std::runtime_error("ERROR");
              //throw "ERROR, the immediate hex value for this .word instruction is not in range!";
            }
          }
          else {
            if (hexValue < 0 || hexValue > 65535) {
              currentNode = nullptr;
              /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
              "\nBranch offset out of range");*/
              throw std::runtime_error("ERROR");
              //throw "ERROR, the immdediate hex value for this instruction is not in range!";
            }
          }
          intermediateRepresentation.emplace_back(currentToken);
        }
        else if (currentTokenKind == Token::WORD || currentTokenKind == Token::ID) {
          intermediateRepresentation.emplace_back(currentToken);
        }
      }
      if (!currentNode->isFinishingNode()) {
        currentNode = nullptr;
        /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
        "\nThis instruction of type: " + instructionType + " is not complete");*/
        throw std::runtime_error("ERROR");
      }
      currentNode = nullptr;
      if (instructionType == "labelOnly") {
        continue; // we do not need to update intermediateRepresentations or locationCounter because this is a line with only labels on it
      }
      intermediateRepresentations.emplace_back(intermediateRepresentation);
      locationCounter += 4;

      // This code is just an example - you don't have to use a range-based
      // for loop in your actual assembler. You might find it easier to
      // use an index-based loop like this:
      // for(int i=0; i<tokenLine.size(); i++) { ... }
      //for (auto &token : tokenLine) {
      //  std::cout << token << ' ';
      //}

      // Remove this when you're done playing with the example program!
      // Printing a random newline character as part of your machine code
      // output will cause you to fail the Marmoset tests.
      //std::cout << std::endl;

    } // end of while loop that does the first scan of all the lines

    // Synthesis stage where we produce output, note we can still throw errors at this stage, in particular our concern is .word and beq/bne instructions
    // where label usage is incorrect, either label offset calcualtion is out of range or we try to jump to a label that has not been defined

    locationCounter = 0; // reset locationCounter as we are now doing a second pass only through our intermediate representations

    for (int i = 0; i < intermediateRepresentations.size(); i++) {
      std::vector<Token> currentRepresentation = intermediateRepresentations.at(i);
      Token currentInstruction = currentRepresentation.at(0);
      // storing all the lexemes of the current intermediate representation in this string, its only use is for descriptive error message in try-catch when throwing runtime errors
      /*std::string currentLineText = "";
      for (int i = 0; i < currentRepresentation.size(); i++) {
        currentLineText += currentRepresentation.at(i).getLexeme() + " ";
      }*/
      //std::cout << currentLineText << std::endl;
      if (currentInstruction.getLexeme() == "add" || currentInstruction.getLexeme() == "sub" || currentInstruction.getLexeme() == "slt" || 
          currentInstruction.getLexeme() == "sltu") {
        int functionStartBits = 0;
        int firstRegisterValue = (int) currentRepresentation.at(2).toNumber();
        int secondRegisterValue = (int) currentRepresentation.at(3).toNumber();
        int destinationRegisterValue = (int) currentRepresentation.at(1).toNumber();
        int opcodeInstruction;
        if (currentInstruction.getLexeme() == "add") {
          opcodeInstruction = 32;
        }
        else if (currentInstruction.getLexeme() == "sub") {
          opcodeInstruction = 34;
        }
        else if (currentInstruction.getLexeme() == "slt") {
          opcodeInstruction = 42;
        }
        else {
          opcodeInstruction = 43; // sltu
        }
        int word = (functionStartBits << 26) | (firstRegisterValue << 21) | (secondRegisterValue << 16) | (destinationRegisterValue << 11) | opcodeInstruction;
        putchar((word >> 24) & 0xff);
        putchar((word >> 16) & 0xff);
        putchar((word >> 8) & 0xff);
        putchar(word & 0xff);
      }
      else if (currentInstruction.getLexeme() == "mult" || currentInstruction.getLexeme() == "multu" || currentInstruction.getLexeme() == "div" 
              || currentInstruction.getLexeme() == "divu") {
        int functionStartBits = 0;
        int firstRegisterValue = (int) currentRepresentation.at(1).toNumber();
        int secondRegisterValue = (int) currentRepresentation.at(2).toNumber();
        int opcodeInstruction;
        if (currentInstruction.getLexeme() == "mult") {
          opcodeInstruction = 24;
        }
        else if (currentInstruction.getLexeme() == "multu") {
          opcodeInstruction = 25;
        }
        else if (currentInstruction.getLexeme() == "div") {
          opcodeInstruction = 26;
        }
        else {
          opcodeInstruction = 27; // divu
        }
        int word = (functionStartBits << 26) | (firstRegisterValue << 21) | (secondRegisterValue << 16) | opcodeInstruction;
        putchar((word >> 24) & 0xff);
        putchar((word >> 16) & 0xff);
        putchar((word >> 8) & 0xff);
        putchar(word & 0xff);
      }
      else if (currentInstruction.getLexeme() == "mfhi" || currentInstruction.getLexeme() == "mflo" || currentInstruction.getLexeme() == "lis") {
        int functionStartBits = 0;
        int registerValue = (int) currentRepresentation.at(1).toNumber();
        int opcodeInstruction;
        if (currentInstruction.getLexeme() == "mfhi") {
          opcodeInstruction = 16;
        }
        else if (currentInstruction.getLexeme() == "mflo") {
          opcodeInstruction = 18;
        }
        else {
          opcodeInstruction = 20; // lis
        }
        int word = (functionStartBits << 16) | (registerValue << 11) |  opcodeInstruction;
        putchar((word >> 24) & 0xff);
        putchar((word >> 16) & 0xff);
        putchar((word >> 8) & 0xff);
        putchar(word & 0xff);
      }
      else if (currentInstruction.getLexeme() == "jr" || currentInstruction.getLexeme() == "jalr") {
        int functionStartBits = 0;
        int registerValue = (int) currentRepresentation.at(1).toNumber();
        int opcodeInstruction;
        if (currentInstruction.getLexeme() == "jr") {
          opcodeInstruction = 8;
        }
        else {
          opcodeInstruction = 9;
        }
        int word = (functionStartBits << 26) | (registerValue << 21) |  opcodeInstruction;
        putchar((word >> 24) & 0xff);
        putchar((word >> 16) & 0xff);
        putchar((word >> 8) & 0xff);
        putchar(word & 0xff);
      }
      else if (currentInstruction.getLexeme() == "lw" || currentInstruction.getLexeme() == "sw") {
        int firstRegisterValue = (int) currentRepresentation.at(1).toNumber();
        int immediateValue = (int) currentRepresentation.at(2).toNumber();
        int secondRegisterValue = (int) currentRepresentation.at(3).toNumber();
        int functionStartBits;
        if (currentInstruction.getLexeme() == "lw") {
          functionStartBits = 35;
        }
        else {
          functionStartBits = 43; // sw
        }
        int word = (functionStartBits << 26) | (secondRegisterValue << 21) |  (firstRegisterValue << 16) | (immediateValue & 0xffff);
        putchar((word >> 24) & 0xff);
        putchar((word >> 16) & 0xff);
        putchar((word >> 8) & 0xff);
        putchar(word & 0xff);
      }
      else if (currentInstruction.getLexeme() == "beq" || currentInstruction.getLexeme() == "bne") {
        int immediateValue;
        if (currentRepresentation.at(3).getKind() == Token::ID) {
          std::string labelImmediate = currentRepresentation.at(3).getLexeme() + ":"; // add the ":" to match label definition exactly; these are the keys to symbolTable
          if (symbolTable.count(labelImmediate) == 0) {
            /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
            "\nNo such label: " + currentRepresentation.at(3).getLexeme());*/
            throw std::runtime_error("ERROR");
            //throw "ERROR, cannot jump to label: " + labelImmediate + ", it has not been defined anywhere!";
          }
          immediateValue = symbolTable.at(labelImmediate);
          //int offsetValue = (symbolTable.at(labelImmediate) - locationCounter - 4) / 4;
          immediateValue = (immediateValue - locationCounter - 4) / 4;
          if (immediateValue > 32767 || immediateValue < -32768) {
            /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
            "\nBranch offset out of range");*/
            throw std::runtime_error("ERROR");
            //throw "ERROR, the label offset: " + labelImmediate + "is out of range!";
          }
        }
        else {
          immediateValue = (int) currentRepresentation.at(3).toNumber();
        }
        int firstRegisterValue = (int) currentRepresentation.at(1).toNumber();
        int secondRegisterValue = (int) currentRepresentation.at(2).toNumber();
        int functionStartBits;
        if (currentInstruction.getLexeme() == "beq") {
          functionStartBits = 4;
        }
        else {
          functionStartBits = 5; // bne
        }
        int word = (functionStartBits << 26) | (firstRegisterValue << 21) |  (secondRegisterValue << 16) | (immediateValue & 0xffff);
        putchar((word >> 24) & 0xff);
        putchar((word >> 16) & 0xff);
        putchar((word >> 8) & 0xff);
        putchar(word & 0xff);
      }
      else {
        // .word instruction
        int immediateValue;
        if (currentRepresentation.at(1).getKind() == Token::ID) {
          std::string labelImmediate = currentRepresentation.at(1).getLexeme() + ":"; // add the ":" to match label definition exactly; these are the keys to symbolTable
          if (symbolTable.count(labelImmediate) == 0) {
            /*throw std::runtime_error("ERROR: Parse error in line: " + currentLineText + 
            "\nNo such label: " + currentRepresentation.at(1).getLexeme());*/
            throw std::runtime_error("ERROR");
            //throw "ERROR, cannot jump to label: " + labelImmediate + ", it has not been defined anywhere!";
          }
          immediateValue = symbolTable.at(labelImmediate);
        }
        else {
          immediateValue = (int) currentRepresentation.at(1).toNumber();
        }
        int word = immediateValue;
        putchar((word >> 24) & 0xff);
        putchar((word >> 16) & 0xff);
        putchar((word >> 8) & 0xff);
        putchar(word & 0xff);
      }
      locationCounter += 4;
    }
  } 
  catch (ScanningFailure &f) {
    // deallocate the LinkedLists here as program won't reach intented deallocation component
    
    // let's deal with the lw/sw instruction LinkedList first 
    delete lparenLwSwWithLabel;
    hexIntLwSwWithLabel->clearNextNodes();
    intLwSwWithLabel->clearNextNodes();

    for (int i = 0; i < instructionsWithLabels.size(); i++) {
      delete instructionsWithLabels.at(i);
      instructionsWithLabels.at(i) = nullptr;
    }
    for (int i = 0; i < instructionsNoLabels.size(); i++) {
      instructionsNoLabels.at(i) = nullptr;
    }
    std::cerr << f.what() << std::endl;
    return 1;
  }
  // had catch (const char* errorMessage) before changed to runtime_error cuz its better 
  catch (std::runtime_error& e) {
    // deallocate the LinkedLists here as program won't reach intented deallocation component
    
    // let's deal with the lw/sw instruction LinkedList first 
    delete lparenLwSwWithLabel;
    hexIntLwSwWithLabel->clearNextNodes();
    intLwSwWithLabel->clearNextNodes();

    for (int i = 0; i < instructionsWithLabels.size(); i++) {
      delete instructionsWithLabels.at(i);
      instructionsWithLabels.at(i) = nullptr;
    }
    for (int i = 0; i < instructionsNoLabels.size(); i++) {
      instructionsNoLabels.at(i) = nullptr;
    }
    std::cerr << e.what() << std::endl;
    return 1;
  }
  // You can add your own catch clause(s) for other kinds of errors.
  // Throwing exceptions and catching them is the recommended way to
  // handle errors and terminate the program cleanly in C++. Do not
  // use the std::exit function, which may leak memory.
  
  // deallocate the LinkedLists here as intented

  // let's deal with the lw/sw instruction LinkedList first 
  delete lparenLwSwWithLabel;
  hexIntLwSwWithLabel->clearNextNodes();
  intLwSwWithLabel->clearNextNodes();

  for (int i = 0; i < instructionsWithLabels.size(); i++) {
    delete instructionsWithLabels.at(i);
    instructionsWithLabels.at(i) = nullptr;
  }
  for (int i = 0; i < instructionsNoLabels.size(); i++) {
    instructionsNoLabels.at(i) = nullptr;
  }
  return 0;
}