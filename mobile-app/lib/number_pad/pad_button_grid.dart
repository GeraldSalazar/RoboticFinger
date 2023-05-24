import 'package:flutter/material.dart';
import 'number_button.dart';

class NumberPad extends StatelessWidget {
  final Function(String) onNumberTap;
  final Function() deleteLastDigit;
  final Function() checkCode;
  const NumberPad({super.key, required this.onNumberTap, required this.deleteLastDigit, required this.checkCode});

  @override
  Widget build(BuildContext context) {
    return GridView.count(
      padding: const EdgeInsets.all(5),
      crossAxisCount: 3,
      shrinkWrap: true,
      physics: const NeverScrollableScrollPhysics(),
      children: List.generate(9, (index) {
        int number = index + 1;
        return PadButton(
          number: number,
          onNumberTap: onNumberTap,
        );
      })
        ..add(PadButton(number: 0, onNumberTap: onNumberTap))
        ..add(
          Container(
            margin: const EdgeInsets.all(5),
            child: Ink(
              decoration: BoxDecoration(
                  color: Colors.green[100],
                  borderRadius: BorderRadius.circular(20)),
              child: InkWell(
                onTap: () {
                  checkCode();
                },
                child: const Center(
                  child: Text(
                    'Enter',
                    style: TextStyle(fontSize: 24.0),
                  ),
                ),
              ),
            ),
          ),
        )
        ..add(
          Container(
            margin: const EdgeInsets.all(5),
            child: Ink(
              decoration: BoxDecoration(
                  color: Colors.red[100],
                  borderRadius: BorderRadius.circular(20)),
              child: InkWell(
                onTap: () {
                  deleteLastDigit();
                },
                child: const Center(
                  child: Icon(Icons.arrow_back_ios_new_outlined)
                ),
              ),
            ),
          ),
        ),
    );
  }
}
