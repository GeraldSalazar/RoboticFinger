import 'package:digital_num_pad/input_screen/size_selection_widget.dart';
import 'package:digital_num_pad/input_screen/title_widget.dart';
import 'package:digital_num_pad/number_pad/num_pad_screen.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

class InputScreen extends StatelessWidget {
  const InputScreen({super.key});

  @override
  Widget build(BuildContext context) {
    return Scaffold(
          appBar: AppBar(
            backgroundColor: Colors.grey,
            title: const Text('Digital Num Pad'),
          ),
          body: const Center(
            child: Padding(
              padding: EdgeInsets.all(16.0),
              child: NumberCodeInput(),
            ),
        ),
      );
  }
}

class NumberCodeInput extends StatefulWidget {
  const NumberCodeInput({super.key});

  @override
  State<NumberCodeInput> createState() => _NumberCodeInputState();
}

class _NumberCodeInputState extends State<NumberCodeInput> {
  final TextEditingController _controller = TextEditingController();
  String selectedSize = 'Large';

  void _handleSizeSelection(String size) {
    setState(() {
      selectedSize = size;
    });
  }
  @override
  void dispose() {
    _controller.dispose();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      mainAxisSize: MainAxisSize.min,
      children: [
        Container(
          height: MediaQuery.of(context).size.height * 0.30,
          child: TitleWidget(),
        ),
        TextField(
          controller: _controller,
          keyboardType: TextInputType.number,
          inputFormatters: [FilteringTextInputFormatter.digitsOnly],
          decoration: const InputDecoration(
            labelText: 'Enter the code',
          ),
        ),
        const SizedBox(height: 30,),
        SizeSelectionWidget(onSelectSize: _handleSizeSelection),
        const SizedBox(height: 30,),
        FractionallySizedBox(
          widthFactor: 0.5,
          child: ElevatedButton(
            style: ButtonStyle(
              backgroundColor: MaterialStateProperty.all<Color>(Colors.indigo[200]!),
              textStyle: MaterialStateProperty.all<TextStyle>(
                const TextStyle(fontSize: 22),
              ),
            ),
            onPressed: () {
              // Perform code verification logic
              String code = _controller.text;
              if(code.isNotEmpty) {
                Navigator.push(context, MaterialPageRoute(
                    builder: (context) => NumPadScreen(inputCode: code, selectedSize: selectedSize,)));
              } else {
                showDialog(
                  context: context,
                  builder: (BuildContext context) {
                    return AlertDialog(
                      title: const Text('No code provided'),
                      content: const Text("Please type a valid code"),
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
            },
            child: const Text('Start'),
          ),
        ),
      ],
    );
  }
}
