import 'package:flutter/material.dart';
import 'pad_button_grid.dart';
class NumPadScreen extends StatefulWidget {
  String inputCode = '';
  String selectedSize = '';
  NumPadScreen({Key? key, required this.inputCode, required this.selectedSize}) : super(key: key);

  @override
  State<NumPadScreen> createState() => _NumPadScreenState();
}

class _NumPadScreenState extends State<NumPadScreen> {
  String currentCode = '';
  double scaleValue = 1.0;

  void handleNumberTap(String number) {
    setState(() {
      currentCode += number;
    });
    print(widget.selectedSize);
  }
  void deleteLastDigit() {
    setState(() {
      if(currentCode.isNotEmpty) {
        currentCode = currentCode.substring(0, currentCode.length - 1);
      }
    });
  }
  void checkCode(){
    String message = '';
    if(currentCode == widget.inputCode){
      message = 'The code matches!';
    }else{
      message = 'The code is not correct!';
    }
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: const Text('Code check'),
          content: Text(message),
          actions: <Widget>[
            TextButton(
              child: const Text('OK'),
              onPressed: () {
                Navigator.of(context).pop();
              },
            ),
          ],
        );
      },
    );
  }
  @override
  Widget build(BuildContext context) {
    if(widget.selectedSize == 'Medium') {
      scaleValue = 0.85;
    }else if(widget.selectedSize == 'Small'){
      scaleValue = 0.75;
    }
    return Scaffold(
        appBar: AppBar(
          backgroundColor: Colors.grey,
          title: const Text('Digital Num Pad'),
        ),
        body: SingleChildScrollView(
          child: Column(
            children: [
              Padding(
                padding: const EdgeInsets.only(left: 0, right: 0, bottom: 25, top: 40),
                child: Text(currentCode, style: const TextStyle(fontSize: 22),),
              ),
              Transform.scale(scale: scaleValue, child: NumberPad(onNumberTap: handleNumberTap, deleteLastDigit: deleteLastDigit, checkCode: checkCode,)),
            ],
          ),
        )
    );
  }
}
