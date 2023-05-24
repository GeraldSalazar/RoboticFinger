import 'package:flutter/material.dart';

class PadButton extends StatefulWidget {
  final Function(String) onNumberTap;
  final int number;
  const PadButton({super.key, required this.number, required this.onNumberTap});


  @override
  State<PadButton> createState() => _PadButtonState();
}

class _PadButtonState extends State<PadButton> {

  @override
  Widget build(BuildContext context) {
    return Container(
      margin: const EdgeInsets.all(5),
      child: Ink(
        decoration: BoxDecoration(color: Colors.grey[100], borderRadius: BorderRadius.circular(20)),
        child: InkWell(
          onTap: () {
            widget.onNumberTap(widget.number.toString());
          },
          child: Center(
            child: Text(
              widget.number.toString(),
              style: const TextStyle(fontSize: 28.0),
            ),
          ),
        ),
      ),
    );
  }
}

